#include <AICombat/HealerStateMachine.hpp>

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
        ScriptConf healerStateMachine = {};
    }

    HealerIdleState::HealerIdleState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void HealerIdleState::Enter() {}

    void HealerIdleState::Update(float)
    {
        if (HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine))
        {
            if (healerStatMachine->FindClosestTarget() != nullptr)
                healerStatMachine->ChangeState(HealerChaseState::Name);
        }
    }

    HealerChaseState::HealerChaseState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void HealerChaseState::Enter()
    {


    }

    void HealerChaseState::Update(float _dt)
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        if (healerStatMachine == nullptr)
            return;

        Canis::Entity* target = healerStatMachine->FindClosestTarget();

        if (target == nullptr)
        {
            healerStatMachine->ChangeState(HealerIdleState::Name);
            return;
        }

        healerStatMachine->FaceTarget(*target);

        if (healerStatMachine->DistanceTo(*target) <= healerStatMachine->GetAttackRange())
        {
            healerStatMachine->ChangeState(HealerHealState::Name);
            return;
        }

        healerStatMachine->MoveTowards(*target, moveSpeed, _dt);
    }

    HealerHealState::HealerHealState(SuperPupUtilities::StateMachine& _stateMachine) :
        State(Name, _stateMachine) {}

    void HealerHealState::Enter()
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);

        // healerStatMachine->StartStaffGlow();
    }

    void HealerHealState::Update(float _dt)
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);
        if (healerStatMachine == nullptr)
            return;

        Canis::Entity* target;

        if (target = healerStatMachine->FindClosestTarget())
            healerStatMachine->FaceTarget(*target);

        const float duration = std::max(attackDuration, 0.001f);

        // Healer SPECIFIC LOGIC

        if(!healStarted)
        {

            healStartTimer += _dt;

            healerStatMachine->staffVisual->GetComponent<PointLight>().intensity += (_dt * 10.0f); // 0.5 * 10 = intensity 5

            if(healStartTimer >= healerStatMachine->healStartDelay)
            {
                // HEALING LOGIC HERE

                healStarted = true;


                // healerStatMachine->ShootEm();

                // reset back to nothing;
                // healStartTimer = 0.0f;
                // healerStatMachine->staffVisual->GetComponent<PointLight>().intensity = 0.0f;
            }

        }

        if(healStarted)
        {
            if(!target->active || !target->GetScript<Fighter>() || !target->GetScript<Fighter>()->IsAlive())
            {
                healStarted = false;
                return;
            }

            healTimer += _dt;

            if(healTimer >= 1.0f)
            {
                healTimer = 0.0f;
                if (Fighter* fighter = target->GetScript<Fighter>())
                {
                    fighter->m_currentHealth += healAmount;
                }
                Canis::Debug::Log("I healed him chief!");
            }

            
        }

        if (healerStatMachine->GetStateTime() < duration)
            return;

        if(glm::distance(healerStatMachine->FindClosestTarget()->GetComponent<Transform>().position, healerStatMachine->entity.GetComponent<Transform>().position) <= attackRange)
            return;

        if(!healStarted)
        {

            if (healerStatMachine->FindClosestTarget() != nullptr)
                healerStatMachine->ChangeState(HealerChaseState::Name);
            else
                healerStatMachine->ChangeState(HealerIdleState::Name);
        }
    }

    void HealerHealState::Exit()
    {
        HealerStateMachine* healerStatMachine = dynamic_cast<HealerStateMachine*>(m_stateMachine);

        healStartTimer = 0.0f;
        healerStatMachine->staffVisual->GetComponent<PointLight>().intensity = 0.0f;

    }

    HealerStateMachine::HealerStateMachine(Canis::Entity& _entity) :
        Fighter(_entity),
        idleState(*this),
        chaseState(*this),
        healerHealState(*this) {}

    // std::string HealerStateMachine::GetName()
    // {
    //     return ScriptName;
    // }

    void RegisterHealerStateMachineScript(Canis::App& _app)
    {
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, targetTag);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, detectionRange);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, bodyColliderSize);
        RegisterAccessorProperty(healerStateMachine, AICombat::HealerStateMachine, chaseState, moveSpeed);
        RegisterAccessorProperty(healerStateMachine, AICombat::HealerStateMachine, healerHealState, attackRange);
        RegisterAccessorProperty(healerStateMachine, AICombat::HealerStateMachine, healerHealState, attackDuration);
        RegisterAccessorProperty(healerStateMachine, AICombat::HealerStateMachine, healerHealState, attackDamageTime);
        RegisterAccessorProperty(healerStateMachine, AICombat::HealerStateMachine, healerHealState, healTimer);
        RegisterAccessorProperty(healerStateMachine, AICombat::HealerStateMachine, healerHealState, healStarted);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, maxHealth);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, logStateChanges);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, staffVisual);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, hitSfxPath1);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, hitSfxPath2);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, hitSfxVolume);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, healStartDelay);
        REGISTER_PROPERTY(healerStateMachine, AICombat::HealerStateMachine, deathEffectPrefab);

        DEFAULT_CONFIG_AND_REQUIRED(
            healerStateMachine,
            AICombat::HealerStateMachine,
            Canis::Transform,
            Canis::Material,
            Canis::Model,
            Canis::Rigidbody,
            Canis::BoxCollider);

        healerStateMachine.DEFAULT_DRAW_INSPECTOR(AICombat::HealerStateMachine);

        _app.RegisterScript(healerStateMachine);
    }

    DEFAULT_UNREGISTER_SCRIPT(healerStateMachine, HealerStateMachine)

    void HealerStateMachine::Create()
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

    void HealerStateMachine::Ready()
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

        healerHealState.attackRange = 15.0f;
        m_currentHealth = std::max(maxHealth, 1);
        m_stateTime = 0.0f;
        m_useFirstHitSfx = true;
        

        ClearStates();
        AddState(idleState);
        AddState(chaseState);
        AddState(healerHealState);

        ChangeState(HealerIdleState::Name);
    }

    void HealerStateMachine::Destroy()
    {
        staffVisual = nullptr;
        SuperPupUtilities::StateMachine::Destroy();
    }

    void HealerStateMachine::Update(float _dt)
    {
        if (!IsAlive())
            return;

        m_stateTime += _dt;
        SuperPupUtilities::StateMachine::Update(_dt);
    }

    Canis::Entity* HealerStateMachine::FindClosestTarget() const
    {
        if (entity.tag.empty() || !entity.HasComponent<Canis::Transform>())
            return nullptr;

        const Canis::Transform& transform = entity.GetComponent<Canis::Transform>();
        const Canis::Vector3 origin = transform.GetGlobalPosition();
        Canis::Entity* lowestTarget = nullptr;
        int lowestHealth = INT_MAX;

        for (Canis::Entity* candidate : entity.scene.GetEntitiesWithTag(entity.tag))
        {
            if (candidate == nullptr || candidate == &entity || !candidate->active)
                continue;

            if (!candidate->HasComponent<Canis::Transform>())
                continue;

            if (const Fighter* other = candidate->GetScript<Fighter>())
            {
                if (!other->IsAlive())
                    continue;

                const Canis::Vector3 candidatePosition = candidate->GetComponent<Canis::Transform>().GetGlobalPosition();
                const float distance = glm::distance(origin, candidatePosition);

                if (distance > detectionRange || other->m_currentHealth >= lowestHealth)
                    continue;

                lowestHealth = other->m_currentHealth;
                lowestTarget = candidate;
            }
        }

        return lowestTarget;
    }

    float HealerStateMachine::DistanceTo(const Canis::Entity& _other) const
    {
        if (!entity.HasComponent<Canis::Transform>() || !_other.HasComponent<Canis::Transform>())
            return std::numeric_limits<float>::max();

        const Canis::Vector3 selfPosition = entity.GetComponent<Canis::Transform>().GetGlobalPosition();
        const Canis::Vector3 targetPosition = _other.GetComponent<Canis::Transform>().GetGlobalPosition();
        return glm::distance(selfPosition, targetPosition);
    }

    void HealerStateMachine::FaceTarget(const Canis::Entity& _target)
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

    void HealerStateMachine::MoveTowards(const Canis::Entity& _target, float _speed, float _dt)
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

    void HealerStateMachine::ChangeState(const std::string& _stateName)
    {
        if (SuperPupUtilities::StateMachine::GetCurrentStateName() == _stateName)
            return;

        if (!SuperPupUtilities::StateMachine::ChangeState(_stateName))
            return;

        m_stateTime = 0.0f;

        if (logStateChanges)
            Canis::Debug::Log("%s -> %s", entity.name.c_str(), _stateName.c_str());
    }

    const std::string& HealerStateMachine::GetCurrentStateName() const
    {
        return SuperPupUtilities::StateMachine::GetCurrentStateName();
    }

    float HealerStateMachine::GetStateTime() const
    {
        return m_stateTime;
    }

    std::string_view HealerStateMachine::GetAttackStateName() const
    {
        return HealerHealState::Name;
    }

    float HealerStateMachine::GetAttackDamageTime() const
    {
        return healerHealState.attackDamageTime;
    }

    float HealerStateMachine::GetAttackRange() const
    {
        return healerHealState.attackRange;
    }

    int HealerStateMachine::GetCurrentHealth() const
    {
        return m_currentHealth;
    }

    void HealerStateMachine::TakeDamage(int _damage)
    {
        if (!IsAlive())
            return;

        const int damageToApply = _damage; // Allow negative damage for healing

        if (damageToApply > 0)
            PlayHitSfx();

        m_currentHealth = std::clamp(m_currentHealth - damageToApply, 0, maxHealth);

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

    void HealerStateMachine::PlayHitSfx()
    {
        const Canis::AudioAssetHandle& selectedSfx = m_useFirstHitSfx ? hitSfxPath1 : hitSfxPath2;
        m_useFirstHitSfx = !m_useFirstHitSfx;

        if (selectedSfx.Empty())
            return;

        Canis::AudioManager::PlaySFX(selectedSfx, std::clamp(hitSfxVolume, 0.0f, 1.0f));
    }

    void HealerStateMachine::SpawnDeathEffect()
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

    bool HealerStateMachine::IsAlive() const
    {
        return m_currentHealth > 0;
    }
}
