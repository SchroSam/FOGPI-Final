#include <AICombat/MageBullet.hpp>

#include <AICombat/Fighter.hpp>
#include <Canis/App.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>

namespace AICombat
{
    namespace
    {
        Canis::ScriptConf scriptConf = {};
    }

    void RegisterMageBulletScript(Canis::App& _app)
    {

        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, damage);
        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, speed);
        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, lifeTime);
        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, gravity);
        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, hitImpulse);
        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, collisionMask);
        REGISTER_PROPERTY(scriptConf, AICombat::MageBullet, targetTag);

        DEFAULT_CONFIG(scriptConf, AICombat::MageBullet);

        scriptConf.DEFAULT_DRAW_INSPECTOR(AICombat::MageBullet);

        _app.RegisterScript(scriptConf);
    }

    DEFAULT_UNREGISTER_SCRIPT(scriptConf, MageBullet)

    void MageBullet::CollisionCheck(const Canis::Vector3& _start, const Canis::Vector3& _end)
    {
        const Canis::Vector3 travel = _end - _start;
        const float distance = glm::length(travel);
        if (distance <= 0.0001f)
            return;

        const Canis::Vector3 direction = travel / distance;
        Canis::RaycastHit hit = {};

        if (!entity.scene.Raycast(_start, direction, hit, distance, collisionMask))
            return;

        if (hit.entity == nullptr || hit.entity == &entity)
            return;

        if(hit.entity != nullptr)
            Canis::Debug::Log("%s", hit.entity->name);

        if (IsValidTarget(*hit.entity) && hit.entity->HasComponent<Canis::Rigidbody>())
        {
            //DAMAGE
            Fighter* targetStateMachine = hit.entity->GetScript<Fighter>();
            
            // if (targetStateMachine == nullptr || !targetStateMachine->IsAlive())
            //     continue;

            // if (other->tag != targetTag)
            //     continue;

            Canis::Debug::Log("%s made a hit", entity.name);
            targetStateMachine->TakeDamage(damage);
            entity.Destroy();
        }

        
    }

    void MageBullet::Move(float _dt)
    {
        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        transform.position += transform.GetForward() * speed * _dt;
        transform.position += Canis::Vector3(0.0f, gravity * _dt, 0.0f);
    }

    bool MageBullet::IsValidTarget(const Canis::Entity& _entity) const
    {
        return _entity.tag == targetTag;
    }

    void MageBullet::Create() {}

    void MageBullet::Ready() {}

    void MageBullet::Destroy() {}

    void MageBullet::Update(float _dt) 
    {
        Move(_dt);

        const Canis::Vector3 start = entity.GetComponent<Transform>().GetGlobalPosition();

        CollisionCheck(start, start + entity.GetComponent<Transform>().GetForward());

        m_timeRemaining -= _dt;

        if(m_timeRemaining <= 0.0f)
            entity.Destroy();
    }
}
