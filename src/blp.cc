// generated 2006/3/8 16:31:49 MST by bda@localhost.(none)
// using glademm V2.6.0
//
// newer (non customized) versions of this file go to blp.cc_new

// This file is for your program, I won't touch it again!

#include <config.h>
#include <gtkmm/main.h>
#include <glib/gi18n.h>
#include <hamlib/rig.h>
#include <pthread.h>

#include "windowBLP.hh"
#include "dialogAlert.hh"
#include "windowBLP_glade.hh"
#include "blp.h"
#include "listInit.hh"

windowBLP *windowBLP;
dialogAlert *dialogAlert;

freq_t hlib_freq;
pthread_t hlib;
pthread_mutex_t hlib_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t hlib_cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t hlib_cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char **argv)
{  

  int i;
  int retcode;
  std::vector<std::string> opList;
  std::vector<std::string> bmList;
  std::string logFileName="myTestLog.log";
  std::string dbHost = "";
  std::string dbUser = "root";
  std::string dbPass = "";
  std::string rigPort = "";
  int rigRate = 0;
  bool useLp = true;
  int rigNumber=-1;
  bool displayHelp = 0;
  bool checkFreq = 1;
  

  for(int i = 1; i < argc; ++i) {
    if( strcmp( argv[i], "--help") == 0 )
      displayHelp = 1;
    else if( strcmp( argv[i], "--host") == 0 )
      dbHost=argv[++i];
    else if( strcmp( argv[i], "--user") == 0 )
      dbUser=argv[++i];
    else if( strcmp( argv[i], "--password") == 0 )
      dbPass=argv[++i];
    else if( strcmp( argv[i], "--port") == 0 )
      rigPort=argv[++i];
    else if( strcmp( argv[i], "--rig") == 0 )
      rigNumber=atoi(argv[++i]);
    else if( strcmp( argv[i], "--rate") == 0 )
      rigRate=atoi(argv[++i]);
    else if( strcmp( argv[i], "--log") == 0 )
      logFileName=argv[++i];
    else if( strcmp( argv[i], "--no-lp") == 0 )
      useLp=false;
    else {
      displayHelp = 1;
      break;
    }
  }
  
  if( displayHelp || dbHost == "" ) {
    std::cout << "usage blp --host hostname [--user username] [--password password] [--port port] [--rig rigNumber] [--log logName] [--no-lp] [--rate baud_rate]\n";
    exit(1);
  }

  if( rigNumber < 0 ) {
    std::cout << "No rig specified. Disabling band limit checks.";
    rigNumber = 1;
    checkFreq = 0; // Disable band limit checks
  }

  // Setup Parapin
  if( useLp && pin_init_user(LPT1) ) {
    std::cout << "Could not open lp port. Pass --no-lp to disable.";
  }
  pin_output_mode(LP_DATA_PINS);

  uid_t ruid;
  getresuid(&ruid, NULL, NULL);
  setuid(ruid);

#if defined(ENABLE_NLS)
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif //ENABLE_NLS
   
  Gtk::Main m(&argc, &argv);

  progLog dataLogger(logFileName, LOGALL);
  dataLogger.logData("Program Start", LOGMESSAGES);

  dataAccessMan dam(dbHost, "FieldDay", dbUser, dbPass, &dataLogger);
  contactManager conMan(&dam, checkFreq, useLp);


  windowBLP = new class windowBLP(&conMan);
  dialogAlert = new class dialogAlert();

  windowBLP->hostname = dbHost;

  windowBLP->statusbarBLP->push("Select a New Radio Operator", 0);
  rOpColumns radioOpColumns;
  secColumns sectionColumns;


  Glib::RefPtr<Gtk::ListStore> opRefListStore = Gtk::ListStore::create(radioOpColumns);
  Glib::RefPtr<Gtk::ListStore> sectionRefListStore = Gtk::ListStore::create(sectionColumns);

  windowBLP->comboboxOperator->set_model(opRefListStore);
  windowBLP->comboboxLogger->set_model(opRefListStore);

  Gtk::TreeModel::Row row = *(opRefListStore->append());
  row[radioOpColumns.name_col_text] = "Logout";

  dam.getOperatorList(opList);
  for(i=0; i<opList.size(); i++)
    {
      row = *(opRefListStore->append());
      row[radioOpColumns.name_col_text] = opList.at(i);
    }
  windowBLP->comboboxLogger->pack_start( radioOpColumns.name_col_text);
  windowBLP->comboboxOperator->pack_start( radioOpColumns.name_col_text);

  bmColumns bandModeColumns;
  Glib::RefPtr<Gtk::ListStore> bandModeRefListStore = Gtk::ListStore::create(bandModeColumns);
  windowBLP->comboboxBandMode->set_model(bandModeRefListStore);


  row = *(bandModeRefListStore->append());
  row[bandModeColumns.name_col_text] = "NONE";

  dam.getBandModeList(bmList);
  for(i=0; i<bmList.size(); i++)
    {
      row = *(bandModeRefListStore->append());
      row[bandModeColumns.name_col_text] = bmList.at(i);
    }
  windowBLP->comboboxBandMode->pack_start( bandModeColumns.name_col_text);


  // Setup HamLib
  RIG* theRig;
  theRig = rig_init(rigNumber); 
  if(theRig == NULL) {
    std::cerr << "rig_init failed" << std::endl;
    exit (-1);
  }

  strncpy(theRig->state.rigport.pathname, rigPort.c_str(), FILPATHLEN);
  if(rigRate != 0) // Only set baud rate if it was specified on the command line
    theRig->state.rigport.parm.serial.rate = rigRate;


  retcode=rig_open(theRig);
  if(retcode != RIG_OK) 
  {
    std::cerr << "rig_open failed: " << rigerror(retcode) << std::endl;
    exit(-1);
  }
  
  
  pthread_create(&hlib,NULL,hamlib_poll_thread, (void*) theRig);

  Glib::signal_timeout().connect(sigc::bind(&hamlib_poll, &conMan), 2000);


  m.run(*windowBLP);
  delete windowBLP;
  dam.releaseBandMode();
  rig_close(theRig);
  rig_cleanup(theRig);
  exit(0);
}

bool hamlib_poll(contactManager* theMan)
{
  pthread_mutex_lock(&hlib_mutex);
  theMan->setFrequency(hlib_freq);
  pthread_mutex_unlock(&hlib_mutex);
  pthread_cond_signal(&hlib_cond);
  return true;
}

void *hamlib_poll_thread(void *myPassedRig)
{
  RIG* myRig = (RIG*) myPassedRig;
  freq_t l_freq;
  vfo_t l_vfo;
  pthread_mutex_lock(&hlib_cond_mutex);
  while(pthread_cond_wait(&hlib_cond,&hlib_cond_mutex) == 0){
    rig_get_freq(myRig, RIG_VFO_CURR, &l_freq);
    pthread_mutex_lock(&hlib_mutex);
    hlib_freq = l_freq;
    pthread_mutex_unlock(&hlib_mutex);
  }
  pthread_mutex_unlock(&hlib_cond_mutex);
}
