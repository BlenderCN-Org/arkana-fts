#ifndef D_UPDATEABLE_H
#define D_UPDATEABLE_H

#include <utilities/Singleton.h>

#include <set>
#include <map>

namespace FTS {

class Clock;

/// Interface for the udpate handling of any subsystem. An object which want to
/// be a part of the update procedure should implement this interface and attach
/// to the \a UpdateableManager.
class Updateable {
public:
    /// \return true means keep on being updated, false means stop updating me.
    virtual bool update(const Clock&) = 0;
    virtual ~Updateable() {};
};

class UpdateableManager : public LazySingleton<UpdateableManager> {
    typedef std::map<String, Updateable*> UpdateableMap;
    UpdateableMap m_all;

    std::set<Updateable*> m_anonymous;

    UpdateableManager();
    friend class LazySingleton<UpdateableManager>;
public:
    virtual ~UpdateableManager();

    /// Adds a named item to be updated on every game tick.
    /// \param in_sName The name of the item.
    /// \param in_pUpd The item to be updated.
    /// \return A reference to myself.
    UpdateableManager& add(const String& in_sName, Updateable *in_pUpd);
    /// Removes a named item from the list of items to be updated on every game tick.
    /// \param in_sName The name of the item to remove.
    /// \return A reference to myself.
    UpdateableManager& rem(const String& in_sName);

    /// Adds an anonymous item to be updated on every game tick.
    /// \param in_pUpd The item to be updated.
    /// \return A reference to myself.
    UpdateableManager& add(Updateable *in_pUpd);
    /// Removes an anonymous or named item from the list of items to be updated
    /// on every game tick. Use the named version for named items, it is faster.
    /// \param in_pUpd The item to be removed.
    /// \return A reference to myself.
    UpdateableManager& rem(Updateable *in_pUpd);

    UpdateableManager& doUpdates(const Clock&);
};

}; // namespace FTS

#endif // D_UPDATEABLE_H
