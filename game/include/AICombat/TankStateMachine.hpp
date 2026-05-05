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
    class TankStateMachine;

    class TankIdleState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "IdleState";

        explicit TankIdleState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class TankChaseState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "ChaseState";
        float moveSpeed = 2.0f;

        explicit TankChaseState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class TankHammerTimeState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "TankHammerTimeState";
        float hammerRestDegrees = 140.0f;
        float hammerSwingDegrees = -120.0f;
        float attackRange = 2.25f;
        float attackDuration = 1.5f;
        float attackDamageTime = 0.25f;

        explicit TankHammerTimeState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
        void Exit() override;
    };

    class TankStateMachine : public Fighter
    {
    public:
        static constexpr const char* ScriptName = "AICombat::TankStateMachine";

        Canis::Entity* hammerVisual = nullptr;
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
        float hitSfxVolume = 1.0f;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };

        explicit TankStateMachine(Canis::Entity& _entity);

        TankIdleState idleState;
        TankChaseState chaseState;
        TankHammerTimeState hammerTimeState;

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

        void ResetHammerPose();
        void SetHammerSwing(float _normalized);

        void TakeDamage(int _damage) override;
        bool IsAlive() const override;

    private:
        void PlayHitSfx() override;
        void SpawnDeathEffect() override;
    };

    void RegisterTankStateMachineScript(Canis::App& _app);
    void UnRegisterTankStateMachineScript(Canis::App& _app);
}
