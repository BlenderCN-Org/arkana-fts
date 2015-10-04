#ifndef D_DECORATIVE_MO_H
#define D_DECORATIVE_MO_H

#include "map/MapObject.h"

#include "3d/ModelInstance.h"
#include "utilities/NonCopyable.h"

#include <memory>

namespace FTS {

/// This represents a map object that the user will not have to interact with.
/// The main use of it should be for decorative aspects, like a rock, flowers,
/// a hill, ... All kind of stuff the player will not "click" onto.
class DecorativeMO : public MapObject, public NonCopyable {
public:
    DecorativeMO(std::unique_ptr<ModelInstance> in_pModelInst, const Vector& in_vPos = Vector(), float in_fOrientation = 0.0f, const Vector& in_vScale = Vector(1.0f, 1.0f, 1.0f));
    DecorativeMO(std::unique_ptr<ModelInstance> in_pModelInst, const Vector& in_vPos, const Quaternion& in_qRot, const Vector& in_vScale = Vector(1.0f, 1.0f, 1.0f));

    DecorativeMO(DecorativeMO&&);
    DecorativeMO& operator=(DecorativeMO&&);

    virtual ~DecorativeMO();

    virtual void render(const Color& in_playerColor);

    ModelInstance* getModelInst() const;

private:
    std::unique_ptr<ModelInstance> m_pModelInst;
};

} // namespace FTS

#endif // D_DECORATIVE_MO_H
