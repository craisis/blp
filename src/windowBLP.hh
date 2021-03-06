// generated 2006/3/8 16:31:49 MST by bda@localhost.(none)
// using glademm V2.6.0
//
// newer (non customized) versions of this file go to windowBLP.hh_new

// you might replace
//    class foo : public foo_glade { ... };
// by
//    typedef foo_glade foo;
// if you didn't make any modifications to the widget

#ifndef _WINDOWBLP_HH
#  include "windowBLP_glade.hh"
#  define _WINDOWBLP_HH

#include "blp.h"

class windowBLP : public windowBLP_glade
{  
  contactManager *myMan;
  double m_lower, m_upper, m_current;
        void on_new_operator_activate();
        void on_quit_activate();
        void on_qso_activate();
        void on_operator_name_activate();
        void on_station_activate();
        void on_history_activate();
        void on_score_activate();
        void on_reports_activate();
        void on_about1_activate();
        void on_help_activate();
        void on_comboboxLogger_changed();
        void on_comboboxBandMode_changed();
        void on_comboboxOperator_changed();
        void on_entryCall_changed();
        bool on_entryCall_focus_out_event(GdkEventFocus *ev);
        bool on_entryConClass_focus_out_event(GdkEventFocus *ev);
        void on_treeviewCalls_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        void on_treeviewSections_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        bool on_entrySection_focus_out_event(GdkEventFocus *ev);
        void on_entrySection_changed();
        void on_ClearQSO();
        void on_SaveQSO();
public:
  std::string hostname;
  windowBLP(contactManager*);
  void set_freqs(double lower, double upper, double current);
  void alert(std::string Message);
  void showStatus(std::string newStatus);
  void fillClass(std::string classStr);
  void fillSection(std::string sectionStr);
  void fillCall (std::string callStr);
  void disableEntry();
  void enableEntry();
};
#endif
