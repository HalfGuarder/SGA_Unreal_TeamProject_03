// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_BossMonster_Grux.h"

#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

#include "TFT_AnimInstance_Grux.h"
//#include "TFT_HPBarWidget.h"

#include "TFT_GameInstance.h"
//#include "TFT_UIManager.h"
#include "TFT_SoundManager.h"
#include "TFT_Player.h"

#include "UObject/ConstructorHelpers.h"
#include "AIController.h"
#include "TFT_Boss_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "TimerManager.h"

ATFT_BossMonster_Grux::ATFT_BossMonster_Grux()
{
    _possessionExp = 10;
}

void ATFT_BossMonster_Grux::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = 300.0f;

    // UIMANAGER->OpenWidget(UIType::Tutorial);

    if (GetMesh())
    {
        GetMesh()->SetAnimInstanceClass(UTFT_AnimInstance_Grux::StaticClass());
    }
}

void ATFT_BossMonster_Grux::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (_statCom->IsValidLowLevel())
    {
        _statCom->SetLevelAndInit(1);
        _statCom->_deathDelegate.AddUObject(this, &ATFT_BossMonster_Grux::DeathStart);
    }
}

void ATFT_BossMonster_Grux::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    /*if (_isAttacking && !bIsDashing)
    {
        SetActorLocation(LockedLocation);
        SetActorRotation(LockedRotation);
    }*/
}

void ATFT_BossMonster_Grux::SetMesh(FString path)
{
    _meshCom->SetMesh(path);
}

void ATFT_BossMonster_Grux::AttackStart()
{
    Super::AttackStart();

    SOUNDMANAGER->PlaySound("Grux_Swing", GetActorLocation());
}

void ATFT_BossMonster_Grux::AttackHit_Boss()
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

void ATFT_BossMonster_Grux::Attack_AI()
{
    Super::Attack_AI();

    if (!_isAttacking && _animInstance_Grux != nullptr)
    {
        if (!_animInstance_Grux->Montage_IsPlaying(_animInstance_Grux->_attackMontage) &&
            !_animInstance_Grux->Montage_IsPlaying(_animInstance_Grux->_skillMontage))
        {
            int32 randValue = FMath::RandRange(0, 10);

            if (randValue > 1)
            {
                _animInstance_Grux->bIsDashing = false;

                LockedLocation = GetActorLocation();
                LockedRotation = GetActorRotation();

                _animInstance_Grux->PlayAttackMontage();

                _isAttacking = true;

                _curAttackIndex %= 2;
                _curAttackIndex++;
                _animInstance_Grux->JumpToSection(_curAttackIndex);
            }
            else
            {
                _animInstance_Grux->bIsDashing = true;
                bIsDashing = true;

                _animInstance_Grux->PlaySkillMontage();

                GetCharacterMovement()->MaxWalkSpeed = _dashSpeed;
                GetCharacterMovement()->RotationRate = FRotator(0.0f, 900.0f, 0.0f);
            }

            // _animInstance_Grux->OnMontageEnded.AddDynamic(this, &ATFT_BossMonster_Grux::ResetMovementLock);
        }
    }
}

void ATFT_BossMonster_Grux::AttackEnd()
{
    bIsDashing = false;
    _animInstance_Grux->bIsDashing = false;
    GetCharacterMovement()->MaxWalkSpeed = _walkSpeed;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

float ATFT_BossMonster_Grux::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    return ActualDamage;
}

void ATFT_BossMonster_Grux::DeathStart()
{
    Super::DeathStart();

    if (_animInstance_Grux)
    {
        _animInstance_Grux->PlayDeathMontage();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("_animInstance_Grux is NULL in DeathStart! Check AnimInstance setup."));
        return;
    }

    GetWorldTimerManager().SetTimer(_deathTimerHandle, this, &ATFT_BossMonster_Grux::BossDisable, 2.0f, false);

    // UIMANAGER->CloseWidget(UIType::Tutorial);
}

void ATFT_BossMonster_Grux::BossDisable()
{
    Super::DropItem(MonsterType::Normal);

    PrimaryActorTick.bCanEverTick = false;

    /*if (HpBarWidgetInstance)
    {
        HpBarWidgetInstance->RemoveFromParent();
        HpBarWidgetInstance = nullptr;
    }*/

    DeActive();
}

//void ATFT_BossMonster_Grux::StateCheck()
//{
//    Super::StateCheck();
//
//    if (_statCom->IsDead()) return;
//
//    auto curStates = _stateCom->GetCurStates();
//
//    if (curStates.IsEmpty()) return;
//
//    auto controller = Cast<ATFT_Boss_AIController>(GetController());
//
//    if (controller)
//    {
//        controller->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("IsOnState")), true);
//    }
//
//    /*for (auto state : curStates)
//    {
//        switch (state)
//        {
//        case StateType::Airborne:
//
//            bIsOnState = true;
//            _animInstance_Grux->PlayAirborneMontage();
//            _canMove = false;
//            _stateCom->InitState();
//            GetWorldTimerManager().SetTimer(_stateTimerHandle, this, &ATFT_BossMonster_Grux::EndState, 2.0f, false);
//            break;
//
//        case StateType::Stun:
//            bIsOnState = true;
//            _animInstance_Grux->PlayStunMontage();
//            _canMove = false;
//            _stateCom->InitState();
//            GetWorldTimerManager().SetTimer(_stateTimerHandle, this, &ATFT_BossMonster_Grux::EndState, 2.0f, false);
//            break;
//
//        default:
//            break;
//        }
//    }*/
//}

//void ATFT_BossMonster_Grux::EndState()
//{
//    Super::EndState();
//
//    auto controller = Cast<ATFT_Boss_AIController>(GetController());
//
//    if (controller)
//    {
//        controller->GetBlackboardComponent()->SetValueAsBool(FName(TEXT("IsOnState")), false);
//    }
//
//    bIsOnState = false;
//
//    GetWorldTimerManager().ClearTimer(_stateTimerHandle);
//}

void ATFT_BossMonster_Grux::SetAnimInstanceBind()
{
    Super::SetAnimInstanceBind();

    if (!bAnimBind)
    {
        _animInstance_Grux = Cast<UTFT_AnimInstance_Grux>(GetMesh()->GetAnimInstance());

        if (_animInstance_Grux->IsValidLowLevel())
        {
            _animInstance_Grux->OnMontageEnded.AddDynamic(this, &ATFT_Creature::OnAttackEnded);
            _animInstance_Grux->_attackStartDelegate.AddUObject(this, &ATFT_BossMonster_Grux::AttackStart);
            _animInstance_Grux->_attackHitDelegate.AddUObject(this, &ATFT_BossMonster_Grux::AttackHit_Boss);
            _animInstance_Grux->_attackEndDelegate.AddUObject(this, &ATFT_BossMonster_Grux::AttackEnd);
            _animInstance_Grux->_deathEndDelegate.AddUObject(this, &ATFT_BossMonster_Grux::BossDisable);
            //_animInstance_Grux->_stateMontageEndDelegate.AddUObject(this, &ATFT_BossMonster_Grux::EndState);

            bAnimBind = true;
        }
    }
}

void ATFT_BossMonster_Grux::PreActive()
{
    Super::PreActive();
}

void ATFT_BossMonster_Grux::Active()
{
    Super::Active();

}

void ATFT_BossMonster_Grux::DeActive()
{
    Super::DeActive();

    if (_animInstance_Grux->IsValidLowLevel())
    {
        _animInstance_Grux->OnMontageEnded.Clear();
        _animInstance_Grux->_attackStartDelegate.Clear();
        _animInstance_Grux->_attackHitDelegate.Clear();
        _animInstance_Grux->_attackEndDelegate.Clear();
        _animInstance_Grux->_deathEndDelegate.Clear();
        _animInstance_Grux->_stateMontageEndDelegate.Clear();
    }
}