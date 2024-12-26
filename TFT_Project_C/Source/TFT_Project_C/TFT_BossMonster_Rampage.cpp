// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_BossMonster_Rampage.h"
#include "TFT_AnimInstance_Rampage.h"
//#include "TFT_Widgets/TFT_HPBarWidget.h"
#include "TFT_Player.h"
#include "TFT_StatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Engine/DamageEvents.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EngineUtils.h" 
//#include "TFT_NPC2.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

ATFT_BossMonster_Rampage::ATFT_BossMonster_Rampage()
{
    _possessionExp = 20;
}

void ATFT_BossMonster_Rampage::BeginPlay()
{
    Super::BeginPlay();

    PlayerController = GetWorld()->GetFirstPlayerController();

    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

void ATFT_BossMonster_Rampage::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    _statCom->SetLevelAndInit(201);

    if (_statCom->IsValidLowLevel())
    {
        _statCom->_deathDelegate.AddUObject(this, &ATFT_BossMonster_Rampage::DeathStart);
    }
}

void ATFT_BossMonster_Rampage::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (_isAttacking)
    {
        SetActorLocation(LockedLocation);
    }
}

void ATFT_BossMonster_Rampage::SetMesh(FString path)
{
    _meshCom->SetMesh(path);
}

void ATFT_BossMonster_Rampage::AttackHit_Boss()
{
    TArray<FHitResult> hitResults;
    FCollisionQueryParams params(NAME_None, false, this);

    float attackRange = 500.0f;
    float attackRadius = 400.0f;

    bool bResult = GetWorld()->SweepMultiByChannel(
        hitResults,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * attackRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel6,
        FCollisionShape::MakeSphere(attackRadius),
        params
    );

    FVector vec = GetActorForwardVector() * attackRange;
    FVector center = GetActorLocation() + vec * 0.5f;

    FColor drawColor = FColor::Green;

    if (bResult)
    {
        AActor* targetActor = nullptr;

        // ���͸�: "Player" �±׸� ���� ���͸� ó��
        for (const FHitResult& hitResult : hitResults)
        {
            AActor* hitActor = hitResult.GetActor();
            if (hitActor && hitActor->Tags.Contains(FName("Player")))
            {
                targetActor = hitActor; // "Player" �±׸� ���� ���� �߰�
                break; // ���� ����� "Player"�� Ÿ������ ó��
            }
        }

        if (targetActor)
        {
            // ������ ó��
            float hpRatio = _statCom->BossHPRatio();
            float damageMultiplier = (hpRatio < 0.3f) ? 2.0f : 1.0f;

            float baseDamage = _statCom->GetAttackDamage();
            float damage = baseDamage * damageMultiplier;

            FDamageEvent damageEvent;
            targetActor->TakeDamage(damage, damageEvent, GetController(), this);
            _hitPoint = targetActor->GetActorLocation();
            drawColor = FColor::Red;

            // ���� ȿ�� ����
            /*ATFT_Creature* targetCreature = Cast<ATFT_Creature>(targetActor);
            if (targetCreature != nullptr)
            {
                switch (_curAttackIndex)
                {
                case 1:
                    targetCreature->SetState(StateType::Airborne);
                    break;
                case 2:
                    targetCreature->SetState(StateType::Stun);
                    break;
                case 3:
                    targetCreature->SetState(StateType::Slow);
                    break;
                default:
                    break;
                }
            }*/
        }
    }

    // DrawDebugSphere(GetWorld(), center, attackRadius, 20, drawColor, false, 2.0f);
}

void ATFT_BossMonster_Rampage::Attack_AI()
{
    Super::Attack_AI();

    if (!_isAttacking && _animInstance_Rampage != nullptr)
    {
        if (!_animInstance_Rampage->Montage_IsPlaying(_animInstance_Rampage->_attackMontage) &&
            !_animInstance_Rampage->Montage_IsPlaying(_animInstance_Rampage->_skillMontage) &&
            !_animInstance_Rampage->Montage_IsPlaying(_animInstance_Rampage->_JumpskillMontage))
        {
            LockedLocation = GetActorLocation();

            if (bCanUseJumpSkill)
            {
                _animInstance_Rampage->PlayJumpSkillMontage();
                bCanUseJumpSkill = false;
                GetWorld()->GetTimerManager().SetTimer(JumpSkillCooldownTimerHandle, this, &ATFT_BossMonster_Rampage::ResetJumpSkillCooldown, JumpSkillCooldown, false);
            }
            else if (bCanUseSkill)
            {
                _animInstance_Rampage->PlaySkillMontage();
                bCanUseSkill = false;
                GetWorld()->GetTimerManager().SetTimer(SkillCooldownTimerHandle, this, &ATFT_BossMonster_Rampage::ResetSkillCooldown, SkillCooldown, false);
            }
            else
            {
                _animInstance_Rampage->PlayAttackMontage();
            }

            _isAttacking = true;

            _curAttackIndex %= 3;
            _curAttackIndex++;
            _animInstance_Rampage->JumpToSection(_curAttackIndex);

            _animInstance_Rampage->OnMontageEnded.AddDynamic(this, &ATFT_BossMonster_Rampage::ResetMovementLock);
        }
    }

}

void ATFT_BossMonster_Rampage::ResetSkillCooldown()
{
    bCanUseSkill = true;
}

void ATFT_BossMonster_Rampage::ResetJumpSkillCooldown()
{
    bCanUseJumpSkill = true;
}

void ATFT_BossMonster_Rampage::AttackEnd()
{
}

float ATFT_BossMonster_Rampage::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);


    return ActualDamage;
}

void ATFT_BossMonster_Rampage::DeathStart()
{
    Super::DeathStart();

    /*if (GetWorld())
    {
        for (TActorIterator<ATFT_NPC2> NPC2Itr(GetWorld()); NPC2Itr; ++NPC2Itr)
        {
            ATFT_NPC2* NPC2 = *NPC2Itr;
            if (NPC2)
            {
                NPC2->ShowNPC();
                UE_LOG(LogTemp, Log, TEXT("Boss died. NPC2 is now visible."));
                break;
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GetWorld() returned nullptr! Cannot find NPC2."));
    }*/

    // _animInstance_Boss->_deathStartDelegate.RemoveAll(this);

    _animInstance_Rampage->PlayDeathMontage();
    GetWorldTimerManager().SetTimer(_deathTimerHandle, this, &ATFT_BossMonster_Rampage::BossDisable, 2.0f, false);
}

void ATFT_BossMonster_Rampage::ResetMovementLock(UAnimMontage* Montage, bool bInterrupted)
{
    _isAttacking = false;

    _animInstance_Rampage->OnMontageEnded.RemoveDynamic(this, &ATFT_BossMonster_Rampage::ResetMovementLock);
}



void ATFT_BossMonster_Rampage::BossDisable()
{
    Super::DropItem(MonsterType::BOSS);

    // this->SetActorHiddenInGame(true);

    //_animInstance_Rampage->_deathEndDelegate.RemoveAll(this);
    //_animInstance_Boss->_attackStartDelegate.RemoveAll(this);
    //_animInstance_Boss->_attackHitDelegate.RemoveAll(this);

    PrimaryActorTick.bCanEverTick = false;
    // auto controller = GetController();
    // if (controller != nullptr) GetController()->UnPossess();

    /*if (HpBarWidgetInstance)
    {
        HpBarWidgetInstance->RemoveFromParent();
        HpBarWidgetInstance = nullptr;
    }*/

    DeActive();
}

void ATFT_BossMonster_Rampage::UpdateBlackboardTarget()
{
}

void ATFT_BossMonster_Rampage::ExplosionHit()
{
    TArray<FHitResult> hitResults;
    FCollisionQueryParams params(NAME_None, false, this);

    float explosionRange = 500.0f; // Explosion ����
    float explosionRadius = 150.0f; // Explosion �ݰ�

    bool bResult = GetWorld()->SweepMultiByChannel(
        hitResults,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * explosionRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel6,
        FCollisionShape::MakeSphere(explosionRadius),
        params
    );

    FVector vec = GetActorForwardVector() * explosionRange;
    FVector center = GetActorLocation() + vec * 0.5f;
    FColor drawColor = FColor::Green;

    // ���� �浹 ó��
    if (bResult)
    {
        for (const FHitResult& hitResult : hitResults)
        {
            if (!hitResult.GetActor() || !hitResult.GetActor()->IsValidLowLevel())
                continue;

            AActor* hitActor = hitResult.GetActor();

            // "Player" �±׸� ���� ���͸� ������ ó��
            if (hitActor->Tags.Contains(FName("Player")))
            {
                drawColor = FColor::Red;
                FDamageEvent damageEvent;

                float explosionDamage = 30.0f; // Explosion ������
                hitActor->TakeDamage(explosionDamage, damageEvent, GetController(), this);
                _hitPoint = hitResult.ImpactPoint;

                UE_LOG(LogTemp, Log, TEXT("Explosion hit player: %s"), *hitActor->GetName());
            }
            else if (hitActor->Tags.Contains(FName("Monster")))
            {
                // "Monster" �±׸� ���� ��� �������� ����
                UE_LOG(LogTemp, Warning, TEXT("Ignored explosion damage on another monster: %s"), *hitActor->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Explosion hit unknown target: %s"), *hitActor->GetName());
            }
        }
    }

    // ����׿� ��ü�� �׷��� Ȯ��
    //DrawDebugSphere(GetWorld(), center, explosionRadius, 20, drawColor, false, 0.1f);
}

void ATFT_BossMonster_Rampage::ChainExplosionHit()
{
    TArray<FHitResult> firstHitResults;
    FCollisionQueryParams params(NAME_None, false, this);

    float chainRange = 600.0f; // Chain Explosion ����
    float chainRadius = 150.0f; // Chain Explosion �ݰ�

    // ù ��° ��ü ó��
    bool bFirstResult = GetWorld()->SweepMultiByChannel(
        firstHitResults,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * chainRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel6,
        FCollisionShape::MakeSphere(chainRadius),
        params
    );

    FVector vec = GetActorForwardVector() * chainRange;
    FVector center = GetActorLocation() + vec * 0.5f;
    FColor drawColor = FColor::Green;

    if (bFirstResult)
    {
        for (const FHitResult& hitResult : firstHitResults)
        {
            if (!hitResult.GetActor() || !hitResult.GetActor()->IsValidLowLevel())
                continue;

            AActor* hitActor = hitResult.GetActor();

            // "Player" �±׸� ���� ���͸� ������ ó��
            if (hitActor->Tags.Contains(FName("Player")))
            {
                drawColor = FColor::Red;
                FDamageEvent damageEvent;

                float chainExplosionDamage = 20.0f; // ù ��° ��ü ������
                hitActor->TakeDamage(chainExplosionDamage, damageEvent, GetController(), this);
                _hitPoint = hitResult.ImpactPoint;

                UE_LOG(LogTemp, Log, TEXT("Chain explosion hit player: %s"), *hitActor->GetName());
            }
            else if (hitActor->Tags.Contains(FName("Monster")))
            {
                UE_LOG(LogTemp, Warning, TEXT("Ignored chain explosion damage on another monster: %s"), *hitActor->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Chain explosion hit unknown target: %s"), *hitActor->GetName());
            }
        }
    }

    // DrawDebugSphere(GetWorld(), center, chainRadius, 20, drawColor, false, 0.1f);

    // �߰� ��ü ó��
    TArray<FHitResult> secondHitResults;
    FVector extendedStartLocation = GetActorLocation() + GetActorForwardVector() * chainRange;
    FVector extendedEndLocation = extendedStartLocation + GetActorForwardVector() * (chainRange * 0.5f);
    FVector secondCenter = (extendedStartLocation + extendedEndLocation) * 0.5f;

    bool bSecondResult = GetWorld()->SweepMultiByChannel(
        secondHitResults,
        extendedStartLocation,
        extendedEndLocation,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel6,
        FCollisionShape::MakeSphere(chainRadius),
        params
    );

    FColor secondDrawColor = FColor::Blue;

    if (bSecondResult)
    {
        for (const FHitResult& secondHitResult : secondHitResults)
        {
            if (!secondHitResult.GetActor() || !secondHitResult.GetActor()->IsValidLowLevel())
                continue;

            AActor* secondHitActor = secondHitResult.GetActor();

            // "Player" �±׸� ���� ���͸� ������ ó��
            if (secondHitActor->Tags.Contains(FName("Player")))
            {
                secondDrawColor = FColor::Red;
                FDamageEvent secondDamageEvent;

                float secondExplosionDamage = 20.0f; // �� ��° ��ü ������
                secondHitActor->TakeDamage(secondExplosionDamage, secondDamageEvent, GetController(), this);

                UE_LOG(LogTemp, Log, TEXT("Second chain explosion hit player: %s"), *secondHitActor->GetName());
            }
            else if (secondHitActor->Tags.Contains(FName("Monster")))
            {
                UE_LOG(LogTemp, Warning, TEXT("Ignored second chain explosion damage on another monster: %s"), *secondHitActor->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Second chain explosion hit unknown target: %s"), *secondHitActor->GetName());
            }
        }
    }

    // DrawDebugSphere(GetWorld(), secondCenter, chainRadius, 20, secondDrawColor, false, 0.1f);
}

void ATFT_BossMonster_Rampage::PreActive()
{
    Super::PreActive();
}

void ATFT_BossMonster_Rampage::SetAnimInstanceBind()
{
    Super::SetAnimInstanceBind();

    if (!bAnimBind)
    {
        _animInstance_Rampage = Cast<UTFT_AnimInstance_Rampage>(GetMesh()->GetAnimInstance());

        if (_animInstance_Rampage->IsValidLowLevel())
        {
            _animInstance_Rampage->OnMontageEnded.AddDynamic(this, &ATFT_Creature::OnAttackEnded);
            _animInstance_Rampage->_attackStartDelegate.AddUObject(this, &ATFT_BossMonster_Rampage::AttackStart);
            _animInstance_Rampage->_attackHitDelegate.AddUObject(this, &ATFT_BossMonster_Rampage::AttackHit_Boss);
            _animInstance_Rampage->_deathEndDelegate.AddUObject(this, &ATFT_BossMonster_Rampage::BossDisable);
            _animInstance_Rampage->OnExplosionHitDelegate.AddUObject(this, &ATFT_BossMonster_Rampage::ExplosionHit);
            _animInstance_Rampage->OnChainExplosionHitDelegate.AddUObject(this, &ATFT_BossMonster_Rampage::ChainExplosionHit);

            bAnimBind = true;
        }
    }
}

void ATFT_BossMonster_Rampage::Active()
{
    Super::Active();
}

void ATFT_BossMonster_Rampage::DeActive()
{
    Super::DeActive();

    if (_animInstance_Rampage->IsValidLowLevel())
    {
        _animInstance_Rampage->OnMontageEnded.Clear();
        _animInstance_Rampage->_attackStartDelegate.Clear();
        _animInstance_Rampage->_attackHitDelegate.Clear();
        _animInstance_Rampage->_deathEndDelegate.Clear();
        _animInstance_Rampage->OnExplosionHitDelegate.Clear();
        _animInstance_Rampage->OnChainExplosionHitDelegate.Clear();
    }
}
