// $Id$
//
// CalFilter - filter for calendar items
//

#ifndef _CALFILTER_H
#define _CALFILTER_H

#include <qstring.h>
#include <qlist.h>

#include "event.h"

namespace KCal {

class CalFilter {
  public:
    CalFilter();
    CalFilter(const QString &name);
    ~CalFilter();
    
    /**
      Set name of filter.
    */
    void setName(const QString &name) { mName = name; }
    /**
      Return name of filter.
    */
    QString name() const { return mName; }
//    QString name() const { return QString("buuuuh"); }
    
    /**
      Apply filter to eventlist, all events not matching filter criterias are
      removed from the list.
    */
    void apply(QList<Event> *eventlist);
    
    /**
      Apply filter criteria on the specified event. Return true, if event passes
      criteria, otherwise return false.
    */
    bool filterEvent(Event *);
    
    /**
      Enable or disable filter.
    */
    void setEnabled(bool);
    /**
      Return wheter the filter is enabled or not.
    */
    bool isEnabled();

    enum { Recurring = 1, Floating = 2 };
    
    /**
      Set criteria, which have to be fulfilled by events passing the filter.
    */
    void setInclusionCriteria(int);
    /**
      Get inclusive filter criteria.
    */
    int inclusionCriteria();
    
    /**
      Set criteria, which prevent events to pass the filter.
    */
    void setExclusionCriteria(int);
    /**
      Get exclusive filter criteria.
    */
    int exclusionCriteria();
    
  private:
    QString mName;

    int mExclusion;
    int mInclusion;
    
    bool mEnabled;
};

}

#endif /*  _CALFILTER_H  */
