#include "Updateable.h"

#include "dLib/dString/dString.h"

using namespace FTS;

UpdateableManager::UpdateableManager()
{
}

UpdateableManager::~UpdateableManager()
{
    // Remove everything that is left in here.
    while(!m_all.empty()) {
        String key = m_all.begin()->first;
        delete m_all.begin()->second;
        // We erase it using the key so that if the updateable's destructor
        // unregisters itself, we won't run into trouble here.
        m_all.erase(key);
    }

    for(std::size_t lastSize = m_anonymous.size() ; !m_anonymous.empty() ; lastSize = m_anonymous.size()) {
        delete *m_anonymous.begin();

        // We remove the entry only in case the destructor didn't do it himself.
        if(m_anonymous.size() == lastSize)
            m_anonymous.erase(m_anonymous.begin());
    }
}

UpdateableManager& UpdateableManager::add(const FTS::String& in_sName, Updateable* in_pUpd)
{
    m_all[in_sName] = in_pUpd;
    return *this;
}

UpdateableManager& UpdateableManager::rem(const FTS::String& in_sName)
{
    m_all.erase(in_sName);
    return *this;
}

UpdateableManager& UpdateableManager::add(Updateable* in_pUpd)
{
    m_anonymous.insert(in_pUpd);
    return *this;
}

UpdateableManager& UpdateableManager::rem(Updateable* in_pUpd)
{
    // First search through the list of anonymous items.
    m_anonymous.erase(in_pUpd);

    // Not found in the anonymous list? search the named list then.
    for(auto i = m_all.begin() ; i != m_all.end() ; ++i) {
        if(i->second == in_pUpd) {
            m_all.erase(i);
            return *this;
        }
    }

    return *this;
}

UpdateableManager& UpdateableManager::doUpdates(const Clock& c)
{
    for(UpdateableMap::iterator i = m_all.begin() ; i != m_all.end() ; ) {
        // When returning false, it means that this updater wants us to stop
        // updating it. We *can* erase the entry while iterating, just the
        // iterator that points to the entry gets invalidated, others stay valid.
        if(!i->second->update(c)) {
            m_all.erase(i++); // Note: post-increment.
        } else {
            ++i;
        }
    }

    for(auto i = m_anonymous.begin() ; i != m_anonymous.end() ; ) {
        // Same story as above.
        if(!(*i)->update(c)) {
            m_anonymous.erase(i++);
        } else {
            ++i;
        }
    }

    return *this;
}
