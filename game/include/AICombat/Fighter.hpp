#pragma once
#include <string>
#include <string_view>
#include <Canis/Entity.hpp>
#include <SuperPupUtilities/StateMachine.hpp>


namespace AICombat
{
    class Fighter : public SuperPupUtilities::StateMachine
    {
    protected:
        virtual void PlayHitSfx() = 0;
        virtual void SpawnDeathEffect() = 0;

        int m_currentHealth = 0;
        float m_stateTime = 0.0f;
        Canis::Vector4 m_baseColor = Canis::Vector4(1.0f);
        bool m_hasBaseColor = false;
        bool m_useFirstHitSfx = true;

    public:
        explicit Fighter(Canis::Entity& _entity);
        virtual ~Fighter() = default;

        std::string targetTag = "";
        float detectionRange = 20.0f;
        Canis::Vector3 bodyColliderSize = Canis::Vector3(1.0f);
        int maxHealth = 40;
        bool logStateChanges = true;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
        float hitSfxVolume = 1.0f;

        virtual Canis::Entity* FindClosestTarget() const = 0;
        virtual float DistanceTo(const Canis::Entity& _other) const = 0;
        virtual void FaceTarget(const Canis::Entity& _target) = 0;
        virtual void MoveTowards(const Canis::Entity& _target, float _speed, float _dt) = 0;
        virtual void ChangeState(const std::string& _stateName) = 0;
        virtual const std::string& GetCurrentStateName() const = 0;
        virtual float GetStateTime() const = 0;
        virtual float GetAttackRange() const = 0;
        virtual int GetCurrentHealth() const = 0;
        virtual std::string_view GetAttackStateName() const = 0;
        virtual float GetAttackDamageTime() const = 0;

        virtual void TakeDamage(int _damage) = 0;
        virtual bool IsAlive() const = 0;
        // end old

        // void Create() override;
        // void Ready() override;
        // void Destroy() override;
        // void Update(float _dt) override;
    };

    inline Fighter::Fighter(Canis::Entity& _entity) : SuperPupUtilities::StateMachine(_entity) {}
}
