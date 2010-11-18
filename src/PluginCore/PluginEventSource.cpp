/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Nov 24, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#include "PluginEvent.h"
#include "PluginEventSink.h"
#include "PluginEventSource.h"
#include "PluginEvents/AttachedEvent.h"

using namespace FB;

PluginEventSource::PluginEventSource()
{
}

PluginEventSource::~PluginEventSource()
{
}

void PluginEventSource::AttachObserver(FB::PluginEventSink *sink)
{
    AttachObserver(sink->shared_ptr());
}

void PluginEventSource::AttachObserver( PluginEventSinkPtr sink )
{
    boost::recursive_mutex::scoped_lock _l(m_observerLock);
    m_observers.push_back(sink);
    AttachedEvent newEvent;
    sink->HandleEvent(&newEvent, this);
}

void PluginEventSource::DetachObserver(FB::PluginEventSink *sink)
{
    DetachObserver(sink->shared_ptr());
}

void PluginEventSource::DetachObserver( PluginEventSinkPtr sink )
{
    boost::recursive_mutex::scoped_lock _l(m_observerLock);
    for (ObserverMap::iterator it = m_observers.begin(); it != m_observers.end(); it++) {
        PluginEventSinkPtr tmp = it->lock();
        if (!tmp) {
            m_observers.erase(it); // Clear out weak_ptrs that don't point to anything alive
        } else if (tmp == sink) {
            m_observers.erase(it);
            DetachedEvent newEvent;
            sink->HandleEvent(&newEvent, this);
            return;
        }
    }
}

bool PluginEventSource::SendEvent(PluginEvent* evt)
{
    boost::recursive_mutex::scoped_lock _l(m_observerLock);
    for (ObserverMap::iterator it = m_observers.begin(); it != m_observers.end(); it++) {
        PluginEventSinkPtr tmp = it->lock();
        if (!tmp) m_observers.erase(it); // Clear out weak_ptrs that don't point to anything alive
        else if (tmp && tmp->HandleEvent(evt, this)) {
            return true;    // Tell the caller that the event was handled
        }
    }
    return false;
}
