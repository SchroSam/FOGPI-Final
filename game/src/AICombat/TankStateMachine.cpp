#include <AICombat/TankStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace AICombat
{
    namespace
    {
        ScriptConf tankStateMachine = {};
    }

    TankIdleState::TankIdleState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void TankIdleState::Enter()
    {
        if (TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine))
            tankStatMachine->ResetHammerPose();
    }

    void TankIdleState::Update(float)
    {
        if (TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine))
        {
            if (tankStatMachine->FindClosestTarget() != nullptr)
                tankStatMachine->ChangeState(TankChaseState::Name);
        }
    }

    TankChaseState::TankChaseState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void TankChaseState::Enter()
    {
        if (TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine))
            tankStatMachine->ResetHammerPose();
    }

    void TankChaseState::Update(float _dt)
    {
        TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine);
        if (tankStatMachine == nullptr)
            return;

        Canis::Entity* target = tankStatMachine->FindClosestTarget();

        if (target == nullptr)
        {
            tankStatMachine->ChangeState(TankIdleState::Name);
            return;
        }

        tankStatMachine->FaceTarget(*target);

        if (tankStatMachine->DistanceTo(*target) <= tankStatMachine->GetAttackRange())
        {
            tankStatMachine->ChangeState(TankHammerTimeState::Name);
            return;
        }

        tankStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    TankHammerTimeState::TankHammerTimeState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void TankHammerTimeState::Enter()
    {
        if (TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine))
            tankStatMachine->SetHammerSwing(0.0f);
    }

    void TankHammerTimeState::Update(float)
    {
        TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine);
        if (tankStatMachine == nullptr)
            return;

        if (Canis::Entity* target = tankStatMachine->FindClosestTarget())
            tankStatMachine->FaceTarget(*target);

        const float duration = std::max(attackDuration, 0.001f);
        tankStatMachine->SetHammerSwing(tankStatMachine->GetStateTime() / duration);

        if (tankStatMachine->GetStateTime() < duration)
            return;

        if (tankStatMachine->FindClosestTarget() != nullptr)
            tankStatMachine->ChangeState(TankChaseState::Name);
        else
            tankStatMachine->ChangeState(TankIdleState::Name);
    }

    void TankHammerTimeState::Exit()
    {
        if (TankStateMachine* tankStatMachine = dynamic_cast<TankStateMachine*>(m_stateMachine))
            tankStatMachine->ResetHammerPose();
    }

    TankStateMachine::TankStateMachine(Canis::Entity& _entity) :
        Fighter(_entity),
        idleState(*this),
        chaseState(*this),
        hammerTimeState(*this) {}

    // std::string TankStateMachine::GetName()
    // {
    //     return ScriptName;
    // }

    void RegisterTankStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, targetTag);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, detectionRange);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, bodyColliderSize);
        RegisterAccessorProperty(tankStateMachine, AICombat::TankStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(tankStateMachine, AICombat::TankStateMachine, hammerTimeState, hammerRestDegrees);
        RegisterAccessorProperty(tankStateMachine, AICombat::TankStateMachine, hammerTimeState, hammerSwingDegrees);
        RegisterAccessorProperty(tankStateMachine, AICombat::TankStateMachine, hammerTimeState, attackRange);
        RegisterAccessorProperty(tankStateMachine, AICombat::TankStateMachine, hammerTimeState, attackDuration);
        RegisterAccessorProperty(tankStateMachine, AICombat::TankStateMachine, hammerTimeState, attackDamageTime);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, maxHealth);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, logStateChanges);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, hammerVisual);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, hitSfxPath1);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, hitSfxPath2);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, hitSfxVolume);
        REGISTER_PROPERTY(tankStateMachine, AICombat::TankStateMachine, deathEffectPrefab);

        DEFAULT_CONFIG_AND_REQUIRED(
            tankStateMachine,
            AICombat::TankStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        tankStateMachine.DEFAULT_DRAW_INSPECTOR(AICombat::TankStateMachine);

        _app.RegisterScript(tankStateMachine);
    }

    DEFAULT_UNREGISTER_SCRIPT(tankStateMachine, TankStateMachine)

    void TankStateMachine::Create()
    {
        entity.GetComponent<Canis::Transform>();

        Canis::Rigidbody& rigidbody = entity.GetComponent<Canis::Rigidbody>();
        rigidbody.motionType = Canis::RigidbodyMotionType::KINEMATIC;
        rigidbody.useGravity = false;
        rigidbody.allowSleeping = false;
        rigidbody.linearVelocity = Canis::Vector3(0.0f);
        rigidbody.angularVelocity = Canis::Vector3(0.0f);

        entity.GetComponent<Canis::BoxCollider>().size = bodyColliderSize;

        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }
    }

    void TankStateMachine::Ready()
    {
        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }

        m_currentHealth = std::max(maxHealth, 1);
        m_stateTime = 0.0f;
        m_useFirstHitSfx = true;
        

        ClearStates();
        AddState(idleState);
        AddState(chaseState);
        AddState(hammerTimeState);

        ResetHammerPose();
        ChangeState(TankIdleState::Name);
    }

    void TankStateMachine::Destroy()
    {
        hammerVisual = nullptr;
        SuperPupUtilities::StateMachine::Destroy();
    }

    void TankStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::StateMachine::Update(_dt);
    }

    Canis::Entity* TankStateMachine::FindClosestTarget() const
    {
        if (targetTag.empty() || !entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* closestTarget = nullptr;
        float closestDistance = detectionRange;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag(targetTag))
        {
            if (candidate == nullptr || candidate == &entity || !candidate->active)
                continue;

            if (!candidate->HasComponent<Canis::Transform>())
                continue;

            if (const TankStateMachine* other = candidate->GetScript<TankStateMachine>())
            {
                if (!other->IsAlive())
                    continue;
            }

            const Canis::Vector3 candidatePosition = candidate->GetComponent<Canis::Transform>().GetGlobalPosition();
            const float distance = glm::distance(origin, candidatePosition);

            if (distance > detectionRange || distance >= closestDistance)
                continue;

            closestDistance = distance;
            closestTarget = candidate;
        }

        return closestTarget;
    }

    float TankStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void TankStateMachine::FaceTarget(const Canis::Entity& _target)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 direction = _target.GetComponent<Canis::Transform>().GetGlobalPosition() - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.rotation.y = std::atan2(-direction.x, -direction.z);
    }

    void TankStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
    {
        if (!entity.HasComponent<Canis::Transform>() || !_target.HasComponent<Canis::Transform>())
            return;

        Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 selfPosition = transform.GetGlobalPosition();
        Canis::Vector3 direction = _target.GetComponent<Canis::Transform>().GetGlobalPosition() - selfPosition;
        direction.y = 0.0f;

        if (glm::dot(direction, direction) <= 0.0001f)
            return;

        direction = glm::normalize(direction);
        transform.position += direction * _speed * _dt;
    }

    void TankStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::StateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::StateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& TankStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::StateMachine::GetCurrentStateName();
    }

    float TankStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    std::string_view TankStateMachine::GetAttackStateName() const
    {
        return TankHammerTimeState::Name;
    }

    float TankStateMachine::GetAttackDamageTime() const
    {
        return hammerTimeState.attackDamageTime;
    }

    float TankStateMachine::GetAttackRange() const
    {
        return hammerTimeState.attackRange;
    }

    int TankStateMachine::GetCurrentHealth() const
    {
        return m_currentHealth;
    }

    void TankStateMachine::ResetHammerPose()
    {
        SetHammerSwing(0.0f);
    }

    void TankStateMachine::SetHammerSwing(float _normalized)
    {
        if (hammerVisual == nullptr || !hammerVisual->HasComponent<Canis::Transform>())
            return;

        Canis::Transform& hammerTransform = hammerVisual->GetComponent<Canis::Transform>();
        const float normalized = Clamp01(_normalized);
        const float swingBlend = (normalized <= 0.5f)
            ? normalized * 2.0f
            : (1.0f - normalized) * 2.0f;

        hammerTransform.rotation.x = DEG2RAD *
            (hammerTimeState.hammerRestDegrees + (hammerTimeState.hammerSwingDegrees * swingBlend));
    }

    void TankStateMachine::TakeDamage(int _damage)
    {
        if (!IsAlive())
            return;

        const int damageToApply = std::max(_damage, 0);

        m_currentHealth = std::max(0, m_currentHealth - damageToApply);
        PlayHitSfx();

        if (m_hasBaseColor && entity.HasComponent<Canis::Material>())
        {
            Canis::Material& material = entity.GetComponent<Canis::Material>();
            const float healthRatio = (maxHealth > 0)
                ? (static_cast<float>(m_currentHealth) / static_cast<float>(maxHealth))
                : 0.0f;

            material.color = Canis::Vector4(
                m_baseColor.x * (0.5f + (0.5f * healthRatio)),
                m_baseColor.y * (0.5f + (0.5f * healthRatio)),
                m_baseColor.z * (0.5f + (0.5f * healthRatio)),
                m_baseColor.w);
        }

        if (m_currentHealth > 0)
            return;

        if (logStateChanges)
            Canis::Debug::Log("%s was defeated.", entity.name.c_str());

        SpawnDeathEffect();
        entity.Destroy();
    }

    void TankStateMachine::PlayHitSfx()
    {
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
    }

    void TankStateMachine::SpawnDeathEffect()
    {
        if (deathEffectPrefab.Empty() || !entity.HasComponent<Canis::Transform>())
            return;

        const Canis::Transform& sourceTransform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 spawnPosition = sourceTransform.GetGlobalPosition();
        const Canis::Vector3 spawnRotation = sourceTransform.GetGlobalRotation();

        for (Canis::Entity* spawnedEntity : entity.scene.Instantiate(deathEffectPrefab))
        {
            if (spawnedEntity == nullptr || !spawnedEntity->HasComponent<Canis::Transform>())
                continue;

            Canis::Transform& spawnedTransform = spawnedEntity->GetComponent<Canis::Transform>();
            spawnedTransform.position = spawnPosition;
            spawnedTransform.rotation = spawnRotation;
        }
    }

    bool TankStateMachine::IsAlive() const
    {
        return m_currentHealth > 0;
    }
}
