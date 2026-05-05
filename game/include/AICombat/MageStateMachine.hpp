#pragma once

#include <Canis/Entity.hpp>
#include <AICombat/Fighter.hpp>
#include <SuperPupUtilities/StateMachine.hpp>
#include <string>

namespace Canis
{
    class App;
}

namespace AICombat
{
    class MageStateMachine;

    class MageIdleState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "IdleState";

        explicit MageIdleState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class MageChaseState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "ChaseState";
        float moveSpeed = 2.0f;

        explicit MageChaseState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class MageAttackState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "MageAttackState";
        float attackRange = 15.0f;
        float attackDuration = 1.5f;
        float attackDamageTime = 0.25f;
        float attackStartTimer = 0.5f;

        explicit MageAttackState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
        void Exit() override;
    };

    class MageStateMachine : public Fighter
    {
    public:
        static constexpr const char* ScriptName = "AICombat::MageStateMachine";

        Canis::Entity* staffVisual = nullptr;
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
        float hitSfxVolume = 1.0f;
        float attackStartDelay = 0.5f;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };
        Canis::SceneAssetHandle bulletPrefab;

        explicit MageStateMachine(Canis::Entity& _entity);

        MageIdleState idleState;
        MageChaseState chaseState;
        MageAttackState mageAttackState;

        void Create() override;
        void Ready() override;
        void Destroy() override;
        void Update(float _dt) override;

        Canis::Entity* FindClosestTarget() const override;
        float DistanceTo(const Canis::Entity& _other) const override;
        void FaceTarget(const Canis::Entity& _target) override;
        void MoveTowards(const Canis::Entity& _target, float _speed, float _dt) override;
        void ChangeState(const std::string& _stateName) override;
        const std::string& GetCurrentStateName() const override;
        float GetStateTime() const override;
        float GetAttackRange() const override;
        int GetCurrentHealth() const override;
        std::string_view GetAttackStateName() const override;
        float GetAttackDamageTime() const override;

        void StartStaffGlow();
        void ShootEm();
        // void SetHammerSwing(float _normalized);

        void TakeDamage(int _damage) override;
        bool IsAlive() const override;

    private:
        void PlayHitSfx() override;
        void SpawnDeathEffect() override;
    };

    void RegisterMageStateMachineScript(Canis::App& _app);
    void UnRegisterMageStateMachineScript(Canis::App& _app);
}
