// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_Boss_BJ.h"
#include "TFT_Player.h"
#include "TFT_AnimInstance_BJ.h"
//#include "TFT_Widgets/TFT_HPBarWidget.h"
#include "TFT_SoundManager.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/SkeletalMeshComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "TFT_GameInstance.h"
//#include "TFT_EffectManager.h"

ATFT_Boss_BJ::ATFT_Boss_BJ()
{
    _possessionExp = 30;
}

void ATFT_Boss_BJ::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

void ATFT_Boss_BJ::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (_statCom->IsValidLowLevel())
    {
        _statCom->SetLevelAndInit(201);
        _statCom->_deathDelegate.AddUObject(this, &ATFT_Boss_BJ::DeathStart);
    }

}

void ATFT_Boss_BJ::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATFT_Boss_BJ::SetMesh(FString path)
{
    _meshCom->SetMesh(path);
}

void ATFT_Boss_BJ::AttackStart()
{
    Super::AttackStart();

    SOUNDMANAGER->PlaySound("Boss_Attack", GetActorLocation());
}

void ATFT_Boss_BJ::FootStep()
{
    Super::FootStep();

    auto player = GetWorld()->GetFirstPlayerController()->GetOwner();

    FVector start = GetActorLocation();
    FRotator rotator = GetActorRotation();
    FVector lineDirAndDist = FVector(1.0f, 1.0f, -100.0f);
    FVector end = start * lineDirAndDist;
    FHitResult hitResult;

    FCollisionQueryParams qParams;
    qParams.AddIgnoredActor(this);
    qParams.bReturnPhysicalMaterial = true;

    GetWorld()->LineTraceSingleByChannel
    (
        hitResult,
        start,
        end,
        ECollisionChannel::ECC_Visibility,
        qParams
    );

    if (hitResult.PhysMaterial != nullptr)
    {

        FString hitName = hitResult.PhysMaterial->GetName();

        // UE_LOG(LogTemp, Log, TEXT("%s"), *hitName);
    }

    if (hitResult.PhysMaterial != nullptr)
    {
        switch (hitResult.PhysMaterial->SurfaceType)
        {
        case SurfaceType1:
            SOUNDMANAGER->FadeOutSound("Boss_FootStep", 0.2f);
            SOUNDMANAGER->PlaySound("Boss_FootStep", end);
            break;
        default:
            break;
        }
    }
}

void ATFT_Boss_BJ::AttackHit_Boss()
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

void ATFT_Boss_BJ::Attack_AI()
{
    Super::Attack_AI();

    if (!_isAttacking && _animInstance_BJ != nullptr)
    {
        if (!_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_attackMontage) &&
            !_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_skillMontage) &&
            !_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_slashMontage))
        {
            int32 randomValue = FMath::RandRange(0, 100);

            if (randomValue < 10)
            {
                if (!_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_skillMontage) &&
                    !_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_slashMontage))
                {
                    _animInstance_BJ->PlaySkillMontage();
                    _isAttacking = true;
                    //SOUNDMANAGER->PlaySound("", GetActorLocation());
                }
            }
            else if (randomValue < 20)
            {
                GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ATFT_Boss_BJ::MoveMessageForward, 1.0f, false);
                _animInstance_BJ->PlaySlashMontage();
                _isAttacking = true;
                SOUNDMANAGER->PlaySound("Boss_Dash", GetActorLocation());
            }
            else if (randomValue < 30)
            {
                if (!_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_skillMontage))
                {
                    ActivateSkill();
                    //SOUNDMANAGER->PlaySound("Boss_Elec", GetActorLocation());
                }
            }
            else
            {
                _animInstance_BJ->PlayAttackMontage();
                _isAttacking = true;
                SOUNDMANAGER->PlaySound("Boss_Attack", GetActorLocation());
            }

            _curAttackIndex %= 4;
            _curAttackIndex++;
            _animInstance_BJ->JumpToSection(_curAttackIndex);
        }
    }
}

void ATFT_Boss_BJ::MoveMessageForward()
{
    FVector currentLocation = GetActorLocation();
    FVector newLocation = currentLocation + GetActorForwardVector() * 500.0f;
    SetActorLocation(newLocation);
}

void ATFT_Boss_BJ::AttackEnd()
{
}

//void ATFT_Boss_BJ::DropItem()
//{
//}

float ATFT_Boss_BJ::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);


    return ActualDamage;
}

void ATFT_Boss_BJ::DeathStart()
{
    Super::DeathStart();

    // _animInstance_BJ->_deathStartDelegate.RemoveAll(this);

    _animInstance_BJ->PlayDeathMontage();

    GetWorldTimerManager().SetTimer(_deathTimerHandle, this, &ATFT_Boss_BJ::BossDisable, 2.0f, false);
}

void ATFT_Boss_BJ::BossDisable()
{
    Super::DropItem(MonsterType::BOSS);

    PrimaryActorTick.bCanEverTick = false;

    /*if (HpBarWidgetInstance)
    {
        HpBarWidgetInstance->RemoveFromParent();
        HpBarWidgetInstance = nullptr;
    }*/

    DeActive();
}

void ATFT_Boss_BJ::SetAnimInstanceBind()
{
    Super::SetAnimInstanceBind();

    if (!bAnimBind)
    {
        _animInstance_BJ = Cast<UTFT_AnimInstance_BJ>(GetMesh()->GetAnimInstance());

        if (_animInstance_BJ->IsValidLowLevel())
        {
            _animInstance_BJ->OnMontageEnded.AddDynamic(this, &ATFT_Creature::OnAttackEnded);
            _animInstance_BJ->_attackStartDelegate.AddUObject(this, &ATFT_Boss_BJ::AttackStart);
            _animInstance_BJ->_attackHitDelegate.AddUObject(this, &ATFT_Boss_BJ::AttackHit_Boss);
            _animInstance_BJ->_deathEndDelegate.AddUObject(this, &ATFT_Boss_BJ::BossDisable);

            bAnimBind = true;
        }
    }
}

void ATFT_Boss_BJ::PreActive()
{
    Super::PreActive();
}

void ATFT_Boss_BJ::Active()
{
    Super::Active();
}

void ATFT_Boss_BJ::DeActive()
{
    Super::DeActive();

    if (_animInstance_BJ->IsValidLowLevel())
    {
        _animInstance_BJ->OnMontageEnded.Clear();
        _animInstance_BJ->_attackStartDelegate.Clear();
        _animInstance_BJ->_attackHitDelegate.Clear();
        _animInstance_BJ->_deathEndDelegate.Clear();
    }
}

void ATFT_Boss_BJ::ActivateSkillRandom()
{
    if (!_isAttacking && _animInstance_BJ != nullptr)
    {
        FVector Origin = GetActorLocation();
        FVector RandomOffset = FVector(FMath::RandRange(-500.f, 500.f), FMath::RandRange(-500.f, 500.f), 0.f);
        TargetLocation = Origin + RandomOffset;

        if (FVector::Dist(TargetLocation, Origin) < 500.f)
        {
            SpawnNiagaraEffectAtLocation(TargetLocation);

            _animInstance_BJ->PlaySkillMontage();
            _isAttacking = true;

            GetWorld()->GetTimerManager().SetTimer(SkillTimerHandle, this, &ATFT_Boss_BJ::TriggerSkillEffect, 2.f, false);
        }
    }
}

void ATFT_Boss_BJ::ActivateSkill()
{
    if (!_isAttacking && _animInstance_BJ != nullptr)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            APawn* PlayerPawn = PlayerController->GetPawn();
            if (PlayerPawn)
            {
                ATFT_Player* Player = Cast<ATFT_Player>(PlayerPawn);
                if (Player)
                {
                    TargetLocation = Player->GetActorLocation();
                    SpawnNiagaraEffectAtLocation(TargetLocation);

                    _animInstance_BJ->PlaySkillMontage();
                    _isAttacking = true;

                    GetWorld()->GetTimerManager().SetTimer(SkillTimerHandle, this, &ATFT_Boss_BJ::TriggerSkillEffect, 2.f, false);
                }
            }
        }
    }
}

void ATFT_Boss_BJ::SpawnNiagaraEffectAtLocation(FVector Location)
{
    UNiagaraSystem* NiagaraEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Script/Niagara.NiagaraSystem'/Game/BeamPack/Niagara/HitPoint/NS_Tornado_Thunder1.NS_Tornado_Thunder1'"));
    if (NiagaraEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraEffect, Location, FRotator::ZeroRotator, FVector(1.0f), true);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load Niagara System for ThunderEffect."));
    }
}

void ATFT_Boss_BJ::TriggerSkillEffect()
{
    _isAttacking = false;

    if (SOUNDMANAGER)
    {
        SOUNDMANAGER->PlaySound("Boss_Elec", GetActorLocation());
    }

    UNiagaraSystem* NiagaraEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Script/Niagara.NiagaraSystem'/Game/Electric_VFX/Niagara/Electric/Boss_Elec.Boss_Elec'"));
    if (NiagaraEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraEffect, TargetLocation);

        TArray<AActor*> ActorsToDamage;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATFT_Player::StaticClass(), ActorsToDamage);

        for (AActor* Actor : ActorsToDamage)
        {
            if (Actor && FVector::Dist(Actor->GetActorLocation(), TargetLocation) < 300.f)
            {
                UGameplayStatics::ApplyDamage(Actor, 20.f, GetController(), this, UDamageType::StaticClass());
            }
        }
    }
}
