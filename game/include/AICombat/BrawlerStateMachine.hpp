#pragma once

#include <Canis/Entity.hpp>
#include <AICombat/Fighter.hpp>

#include <SuperPupUtilities/StateMachine.hpp>

#include <string>

namespace AICombat
{
    class BrawlerStateMachine;

    class IdleState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "IdleState";

        explicit IdleState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class ChaseState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "ChaseState";
        float moveSpeed = 4.0f;

        explicit ChaseState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class HammerTimeState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "HammerTimeState";
        float hammerRestDegrees = 140.0f;
        float hammerSwingDegrees = -120.0f;
        float attackRange = 2.25f;
        float attackDuration = 0.75f;
        float attackDamageTime = 0.25f;

        explicit HammerTimeState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
        void Exit() override;
    };

    class BrawlerStateMachine : public SuperPupUtilities::StateMachine, public Fighter
    {
    protected:
        void PlayHitSfx() override;
        void SpawnDeathEffect() override;

    public:
        static constexpr const char* ScriptName = "AICombat::BrawlerStateMachine";

        Canis::Entity* hammerVisual = nullptr;
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
        float hitSfxVolume = 1.0f;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };

        explicit BrawlerStateMachine(Canis::Entity& _entity);
        //std::string GetName() override;

        IdleState idleState;
        ChaseState chaseState;
        HammerTimeState hammerTimeState;

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

        void TakeDamage(int _damage) override;
        bool IsAlive() const override;

        void ResetHammerPose();
        void SetHammerSwing(float _normalized);

    };

    void RegisterBrawlerStateMachineScript(Canis::App& _app);
    void UnRegisterBrawlerStateMachineScript(Canis::App& _app);
}
