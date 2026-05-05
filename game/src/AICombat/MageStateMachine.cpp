#include <AICombat/MageStateMachine.hpp>

#include <Canis/App.hpp>
#include <Canis/AudioManager.hpp>
#include <Canis/ConfigHelper.hpp>
#include <Canis/Debug.hpp>
#include <AICombat/MageBullet.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace AICombat
{
    namespace
    {
        ScriptConf mageStateMachine = {};
    }

    MageIdleState::MageIdleState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void MageIdleState::Enter()
    {


    }

    void MageIdleState::Update(float)
    {
        if (MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine))
        {
            if (mageStatMachine->FindClosestTarget() != nullptr)
                mageStatMachine->ChangeState(MageChaseState::Name);
        }
    }

    MageChaseState::MageChaseState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void MageChaseState::Enter()
    {


    }

    void MageChaseState::Update(float _dt)
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        if (mageStatMachine == nullptr)
            return;

        Canis::Entity* target = mageStatMachine->FindClosestTarget();

        if (target == nullptr)
        {
            mageStatMachine->ChangeState(MageIdleState::Name);
            return;
        }

        mageStatMachine->FaceTarget(*target);

        if (mageStatMachine->DistanceTo(*target) <= mageStatMachine->GetAttackRange())
        {
            mageStatMachine->ChangeState(MageAttackState::Name);
            return;
        }

        mageStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    MageAttackState::MageAttackState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void MageAttackState::Enter()
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);

        // mageStatMachine->StartStaffGlow();
    }

    void MageAttackState::Update(float _dt)
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);
        if (mageStatMachine == nullptr)
            return;

        if (Canis::Entity* target = mageStatMachine->FindClosestTarget())
            mageStatMachine->FaceTarget(*target);

        const float duration = std::max(attackDuration, 0.001f);

        // MAGE SPECIFIC LOGIC

        attackStartTimer += _dt;

        mageStatMachine->staffVisual->GetComponent<PointLight>().intensity += (_dt * 2.0f);

        if(attackStartTimer >= mageStatMachine->attackStartDelay)
        {
            mageStatMachine->ShootEm();

            // reset back to nothing;
            attackStartTimer = 0.0f;
            mageStatMachine->staffVisual->GetComponent<PointLight>().intensity = 0.0f;
        }

        // END MAGE SPECIFIC

        // mageStatMachine->SetHammerSwing(mageStatMachine->GetStateTime() / duration);

        if (mageStatMachine->GetStateTime() < duration)
            return;

        if (mageStatMachine->FindClosestTarget() != nullptr)
            mageStatMachine->ChangeState(MageChaseState::Name);
        else
            mageStatMachine->ChangeState(MageIdleState::Name);
    }

    void MageAttackState::Exit()
    {
        MageStateMachine* mageStatMachine = dynamic_cast<MageStateMachine*>(m_stateMachine);

        attackStartTimer = 0.0f;
        mageStatMachine->staffVisual->GetComponent<PointLight>().intensity = 0.0f;

    }

    MageStateMachine::MageStateMachine(Canis::Entity& _entity) :
        Fighter(_entity),
        idleState(*this),
        chaseState(*this),
        mageAttackState(*this) {}

    // std::string MageStateMachine::GetName()
    // {
    //     return ScriptName;
    // }

    void RegisterMageStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, targetTag);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, detectionRange);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, bodyColliderSize);
        RegisterAccessorProperty(mageStateMachine, AICombat::MageStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(mageStateMachine, AICombat::MageStateMachine, mageAttackState, attackRange);
        RegisterAccessorProperty(mageStateMachine, AICombat::MageStateMachine, mageAttackState, attackDuration);
        RegisterAccessorProperty(mageStateMachine, AICombat::MageStateMachine, mageAttackState, attackDamageTime);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, maxHealth);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, logStateChanges);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, staffVisual);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, hitSfxPath1);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, hitSfxPath2);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, hitSfxVolume);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, attackStartDelay);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, deathEffectPrefab);
        REGISTER_PROPERTY(mageStateMachine, AICombat::MageStateMachine, bulletPrefab);

        DEFAULT_CONFIG_AND_REQUIRED(
            mageStateMachine,
            AICombat::MageStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        mageStateMachine.DEFAULT_DRAW_INSPECTOR(AICombat::MageStateMachine);

        _app.RegisterScript(mageStateMachine);
    }

    DEFAULT_UNREGISTER_SCRIPT(mageStateMachine, MageStateMachine)

    void MageStateMachine::Create()
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

    void MageStateMachine::Ready()
    {
        if (entity.HasComponent<Canis::Material>())
        {
            m_baseColor = entity.GetComponent<Canis::Material>().color;
            m_hasBaseColor = true;
        }

        // if(entity.tag == "Blue")
        //     bulletPrefab = {.path = "assets/prefabs/mage_bullet_blue.scene"};
        // else
        //     bulletPrefab = {.path = "assets/prefabs/mage_bullet_red.scene"};

        m_currentHealth = std::max(maxHealth, 1);
        m_stateTime = 0.0f;
        m_useFirstHitSfx = true;
        

        ClearStates();
        AddState(idleState);
        AddState(chaseState);
        AddState(mageAttackState);

        ChangeState(MageIdleState::Name);
    }

    void MageStateMachine::Destroy()
    {
        staffVisual = nullptr;
        SuperPupUtilities::StateMachine::Destroy();
    }

    void MageStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::StateMachine::Update(_dt);
    }

    Canis::Entity* MageStateMachine::FindClosestTarget() const
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

            if (const MageStateMachine* other = candidate->GetScript<MageStateMachine>())
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

    float MageStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void MageStateMachine::FaceTarget(const Canis::Entity& _target)
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

    void MageStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
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

    void MageStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::StateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::StateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& MageStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::StateMachine::GetCurrentStateName();
    }

    float MageStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    std::string_view MageStateMachine::GetAttackStateName() const
    {
        return MageAttackState::Name;
    }

    float MageStateMachine::GetAttackDamageTime() const
    {
        return mageAttackState.attackDamageTime;
    }

    float MageStateMachine::GetAttackRange() const
    {
        return mageAttackState.attackRange;
    }

    int MageStateMachine::GetCurrentHealth() const
    {
        return m_currentHealth;
    }

    void MageStateMachine::StartStaffGlow() 
    {
        staffVisual->GetComponent<PointLight>().intensity = 1.0f;
    }

    void MageStateMachine::ShootEm() 
    {
        if (bulletPrefab.Empty() || !entity.HasComponent<Canis::Transform>())
            return;

        const Canis::Transform& sourceTransform = staffVisual->GetComponent<Canis::Transform>();
        const Canis::Vector3 spawnPosition = sourceTransform.GetGlobalPosition();
        const Canis::Vector3 spawnRotation = sourceTransform.GetGlobalRotation();

        for (Canis::Entity* spawnedEntity : entity.scene.Instantiate(bulletPrefab))
        {
            if (spawnedEntity == nullptr || !spawnedEntity->HasComponent<Canis::Transform>())
                continue;

            Canis::Transform& spawnedTransform = spawnedEntity->GetComponent<Canis::Transform>();
            spawnedTransform.position = spawnPosition;
            spawnedTransform.rotation = spawnRotation;

            // give bullet team affiliation
            // if(entity.tag == "Blue")
            //     spawnedEntity->GetComponent<MageBullet>().targetTag = "Red";
            // else
            //     spawnedEntity->GetComponent<MageBullet>().targetTag = "Blue";
            //spawnedEntity->GetComponent<MageBullet>().targetTag = entity.tag == "Blue" ? "Red" : "Blue";
        }
    }

    void MageStateMachine::TakeDamage(int _damage)
    {
        if (!IsAlive())
            return;

        const int damageToApply = std::max(_damage, 0);
        if (damageToApply <= 0)
            return;

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

    void MageStateMachine::PlayHitSfx()
    {
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
    }

    void MageStateMachine::SpawnDeathEffect()
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

    bool MageStateMachine::IsAlive() const
    {
        return m_currentHealth > 0;
    }
}
