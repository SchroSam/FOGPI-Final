#pragma once

#include <Canis/Entity.hpp>
#include <AICombat/Fighter.hpp>
// #include <SuperPupUtilities/StateMachine.hpp>

namespace Canis
{
    class App;
}

namespace AICombat
{
    class HealerStateMachine;

    class HealerIdleState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "IdleState";

        explicit HealerIdleState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class HealerChaseState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "ChaseState";
        float moveSpeed = 4.0f;

        explicit HealerChaseState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
    };

    class HealerHealState : public SuperPupUtilities::State
    {
    public:
        static constexpr const char* Name = "HealerHealState";
        float attackRange = 5.0f;
        float attackDuration = 1.5f;
        float attackDamageTime = 0.25f;
        float healStartTimer = 0.5f;
        float healTimer = 0.0f;
        int healAmount = 2;
        bool healStarted = false;

        explicit HealerHealState(SuperPupUtilities::StateMachine& _stateMachine);
        void Enter() override;
        void Update(float _dt) override;
        void Exit() override;
    };

    class HealerStateMachine : public Fighter
    {
    public:
        static constexpr const char* ScriptName = "AICombat::HealerStateMachine";

        Canis::Entity* staffVisual = nullptr;
        Canis::AudioAssetHandle hitSfxPath1 = { .path = "assets/audio/sfx/hit_1.ogg" };
        Canis::AudioAssetHandle hitSfxPath2 = { .path = "assets/audio/sfx/hit_2.ogg" };
        Canis::AudioAssetHandle healSfx = { .path = "assets/audio/sfx/healing.ogg" };
        float hitSfxVolume = 1.0f;
        float healStartDelay = 0.5f;
        Canis::SceneAssetHandle deathEffectPrefab = { .path = "assets/prefabs/brawler_death_particles.scene" };
        Canis::Entity* healTarget = nullptr;

        explicit HealerStateMachine(Canis::Entity& _entity);

        HealerIdleState idleState;
        HealerChaseState chaseState;
        HealerHealState healerHealState;

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
        // void ShootEm();
        // void SetHammerSwing(float _normalized);

        void TakeDamage(int _damage) override;
        bool IsAlive() const override;

    private:
        void PlayHitSfx() override;
        void SpawnDeathEffect() override;
    };

    void RegisterHealerStateMachineScript(Canis::App& _app);
    void UnRegisterHealerStateMachineScript(Canis::App& _app);
}
