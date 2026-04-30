#pragma once
#include <string>
#include <Canis/Entity.hpp>
#include <SuperPupUtilities/StateMachine.hpp>

namespace Canis
{
    class App;
}

namespace AICombat
{
    class Fighter
    {
    protected:
        virtual void PlayHitSfx();
        virtual void SpawnDeathEffect();

        int m_currentHealth = 0;
        float m_stateTime = 0.0f;
        Canis::Vector4 m_baseColor = Canis::Vector4(1.0f);
        bool m_hasBaseColor = false;
        bool m_useFirstHitSfx = true;

    public:
        static constexpr const char* ScriptName = "AICombat::Fighter";

        //explicit Fighter(Canis::Entity& _entity) : Canis::ScriptableEntity(_entity) {}

        //OLD
        virtual ~Fighter() = default;

        std::string targetTag = "";
        float detectionRange = 20.0f;
        Canis::Vector3 bodyColliderSize = Canis::Vector3(1.0f);
        int maxHealth = 40;
        bool logStateChanges = true;

        virtual Canis::Entity* FindClosestTarget() const;
        virtual float DistanceTo(const Canis::Entity& _other) const;
        virtual void FaceTarget(const Canis::Entity& _target);
        virtual void MoveTowards(const Canis::Entity& _target, float _speed, float _dt);
        virtual void ChangeState(const std::string& _stateName);
        virtual const std::string& GetCurrentStateName() const;
        virtual float GetStateTime() const;
        virtual float GetAttackRange() const;
        virtual int GetCurrentHealth() const;


        virtual void TakeDamage(int _damage);
        virtual bool IsAlive() const;
        // end old

        // void Create() override;
        // void Ready() override;
        // void Destroy() override;
        // void Update(float _dt) override;
    };

    void RegisterFighterScript(Canis::App& _app);
    void UnRegisterFighterScript(Canis::App& _app);
}
