//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_CommunicationUnit_H
#define __INET_CommunicationUnit_H

#include <map>
#include <vector>

#include "INETDefs.h"

#include "ModuleAccess.h"
#include "INotifiable.h"
#include "NotifierConsts.h"

/**
 * Acts as a intermediary between module where state changes can occur and
 * modules which are interested in learning about those changes;
 * "Notification Broker".
 *
 * Notification events are grouped into "categories." Examples of categories
 * are: NF_RADIOSTATE_CHANGED, NF_PP_TX_BEGIN, NF_PP_TX_END, NF_IPv4_ROUTE_ADDED,
 * NF_DATA_LOST, NF_NODE_FAILURE, NF_NODE_RECOVERY, etc. Each category is
 * identified by an integer (right now it's assigned in the source code via an enum,
 * in the future we'll convert to dynamic category registration).
 *
 * To trigger a notification, the client must obtain a pointer to the
 * CommunicationUnit of the given host or router (explained later), and
 * call its fireChangeNotification() method. The notification will be
 * delivered to all subscribed clients immediately, inside the
 * fireChangeNotification() call.
 *
 * Clients that wish to receive notifications should implement (subclass from)
 * the INotifiable interface, obtain a pointer to the CommunicationUnit,
 * and subscribe to the categories they are interested in by calling the
 * subscribe() method of the CommunicationUnit. Notifications will be
 * delivered to the receiveChangeNotification() method of the client
 * (redefined from INotifiable).
 *
 * In cases when the category itself (an int) does not carry enough information
 * about the notification event, one can pass additional information
 * in a data class. There is no restriction on what the data class may contain,
 * except that it has to be subclassed from cObject, and of course
 * producers and consumers of notifications should agree on its contents.
 * If no extra info is needed, one can pass a NULL pointer in the
 * fireChangeNotification() method.
 *
 * A module which implements INotifiable looks like this:
 *
 * <pre>
 * class Foo : public cSimpleModule, public INotifiable {
 *     ...
 *     virtual void receiveChangeNotification(int category, const cObject *details) {..}
 *     ...
 * };
 * </pre>
 *
 * Obtaining a pointer to the CommunicationUnit module of that host/router:
 *
 * <pre>
 * CommunicationUnit *nb; // this is best made a module class member
 * nb = CommunicationUnitAccess().get();  // best done in initialize()
 * </pre>
 *
 *
 * See NED file for additional info.
 *
 * @see INotifiable
 * @author Andras Varga
 */
class INET_API CommunicationUnit : public cSimpleModule
{
  public: // should be protected
    typedef std::vector<INotifiable *> NotifiableVector;
    typedef std::map<int, NotifiableVector> ClientMap;
    friend std::ostream& operator<<(std::ostream&, const NotifiableVector&); // doesn't work in MSVC 6.0

  protected:
    ClientMap clientMap;

  protected:
    /**
     * Initialize.
     */
    virtual void initialize();

    /**
     * Does nothing.
     */
    virtual void handleMessage(cMessage *msg);

  public:
    /** @name Methods for consumers of change notifications */
    //@{
    /**
     * Subscribe to changes of the given category
     */
    virtual void subscribe(INotifiable *client, int category);

    /**
     * Unsubscribe from changes of the given category
     */
    virtual void unsubscribe(INotifiable *client, int category);

    /**
     * Returns true if any client has subscribed to the given category.
     * This, by using a local boolean 'hasSubscriber' flag, allows
     * performance-critical clients to leave out calls to
     * fireChangeNotification() if there's no one subscribed anyway.
     * The flag should be refreshed on each NF_SUBSCRIBERLIST_CHANGED
     * notification.
     */
    virtual bool hasSubscribers(int category);
    //@}

    /** @name Methods for producers of change notifications */
    //@{
    /**
     * Tells CommunicationUnit that a change of the given category has
     * taken place. The optional details object may carry more specific
     * information about the change (e.g. exact location, specific attribute
     * that changed, old value, new value, etc).
     */
    virtual void fireChangeNotification(int category, const cObject *details = NULL);
    //@}
};

/**
 * Gives access to the CommunicationUnit instance within the host/router.
 */
class INET_API CommunicationUnitAccess : public ModuleAccess<CommunicationUnit>
{
  public:
    CommunicationUnitAccess() : ModuleAccess<CommunicationUnit>("communicationUnit") {}
};

#endif




