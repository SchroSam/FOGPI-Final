#pragma once

#include <string>
#include <Canis/Entity.hpp>

namespace Canis
{
    class App;
}

namespace AICombat
{
    class MageBullet : public Canis::ScriptableEntity
    {
    public:
        static constexpr const char* ScriptName = "AICombat::MageBullet";

        int damage = 1;
        float speed = 20.0f;
        float lifeTime = 10.0f;
        float gravity = 0.0f;
        float hitImpulse = 1.0f;
        Canis::Mask collisionMask = Canis::Rigidbody::DefaultMask;
        std::string targetTag = "";


        explicit MageBullet(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;

    private:
        float m_timeRemaining = 0.0f;

        void Move(float _dt);
        void CollisionCheck(const Canis::Vector3& _start, const Canis::Vector3& _end);
        bool IsValidTarget(const Canis::Entity& _entity) const;
    };

    void RegisterMageBulletScript(Canis::App& _app);
    void UnRegisterMageBulletScript(Canis::App& _app);
}
