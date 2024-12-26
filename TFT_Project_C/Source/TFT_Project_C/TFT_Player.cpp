// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_Player.h"

#include "TFT_Creature.h"
#include "TFT_Monster.h"
#include "TFT_HPBarWidget.h"

#include "TFT_AnimInstance_Player.h"
//#include "TFT_NPC.h"
//#include "TFT_Door.h"
//#include "TFT_NPC2.h"
//#include "TFT_Button.h"

#include "EngineUtils.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "TFT_GameInstance.h"
#include "TFT_UIManager.h"
#include "TFT_SoundManager.h"
#include "TFT_EffectManager.h"

//#include "TFT_InvenWidget.h"
//#include "TFT_EquipmentWidget.h"
#include "TFT_Item.h"
#include "TFT_RandomBoxWidget.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"

#include "TFT_Projectile.h"
#include "TFT_Turret.h"
#include "Engine/SkeletalMeshSocket.h"

#include "TFT_SkillUI.h"
#include "TFT_Menu.h"
#include "TFT_RandomBoxWidget.h"

#include "TFT_PlayerSkillComponent.h"

ATFT_Player::ATFT_Player()
{
	_meshCom = CreateDefaultSubobject<UTFT_MeshComponent>(TEXT("Mesh_Com"));

	_invenCom = CreateDefaultSubobject<UTFT_InvenComponent>(TEXT("Inven_Com"));

	_skillCom = CreateDefaultSubobject<UTFT_PlayerSkillComponent>(TEXT("Skill_Com"));

	SetMesh("/Script/Engine.SkeletalMesh'/Game/Blueprints/Characters/Player/Mesh/TFT_SK_SciFi_Soldier_Female_Skin1.TFT_SK_SciFi_Soldier_Female_Skin1'");

	_cameraTLCom = CreateDefaultSubobject<UTimelineComponent>(TEXT("CameraTLComponent"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat> cf
	(TEXT("/Script/Engine.CurveFloat'/Game/Blueprints/Characters/Player/Camera/TFT_Player_Camera_Zoom_Curve.TFT_Player_Camera_Zoom_Curve'"));
	if (cf.Succeeded())
	{
		_cameraZoomCurve = cf.Object;
	}

	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_springArm->SetupAttachment(GetCapsuleComponent());
	_springArm->TargetArmLength = 500.0f;
	_springArm->SetRelativeLocation(FVector(0.0f, 50.0f, 0.0f));
	_springArm->SetRelativeRotation(FRotator(-35.0f, 0.0f, 0.0f));

	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	_camera->SetupAttachment(_springArm);
	_camera->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));

	//GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	////_statCom->SetExp(0);

	static ConstructorHelpers::FClassFinder<UUserWidget> HpBar(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/Widget/Player_HpBar_BP.Player_HpBar_BP_C'"));
	if (HpBar.Succeeded())
	{
		HpBarWidgetClass = HpBar.Class;
	}

	if (HpBarWidgetClass)
	{
		HpBarWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), HpBarWidgetClass);
		if (HpBarWidgetInstance)
		{
			HpBarWidgetInstance->AddToViewport();
		}
	}

	/*static ConstructorHelpers::FObjectFinder<UMaterialInterface> sm
	(TEXT("/Script/Engine.Material'/Game/ParagonSteel/FX/Materials/Heroes/Steel/M_Steel_Armor_ShieldSphere_Impact.M_Steel_Armor_ShieldSphere_Impact'"));
	if (sm.Succeeded())
	{
		_shieldMaterial = sm.Object;
	}*/
	_shield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shield"));
	_shield->SetupAttachment(GetCapsuleComponent());
	_shield->SetMaterial(0, _shieldMaterial);
	_shield->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
	_shield->SetVisibility(false);
	_shield->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	_shieldDashAttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldDashAttackSphere"));
	_shieldDashAttackSphere->SetupAttachment(GetCapsuleComponent());
	_shieldDashAttackSphere->SetupAttachment(_shield);
	_shieldDashAttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FClassFinder<ATFT_Projectile> pc
	(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Characters/Player/Weapons/TFT_Projectile_BP.TFT_Projectile_BP_C'"));
	if (pc.Succeeded())
	{
		_projectileClass = pc.Class;
	}
	/*static ConstructorHelpers::FClassFinder<AActor> lc
	(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Characters/Player/Weapons/TFT_Laser_BP.TFT_Laser_BP_C'"));
	if (lc.Succeeded())
	{
		_laserClass = lc.Class;
	}*/
	/*static ConstructorHelpers::FClassFinder<ATFT_Turret> tc
	(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Characters/Player/Weapons/TFT_Turret_BP.TFT_Turret_BP_C'"));
	if (tc.Succeeded())
	{
		_turretClass = tc.Class;
	}
	static ConstructorHelpers::FClassFinder<ATFT_Turret> tpc
	(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Characters/Player/Weapons/TFT_Turret_Preview_BP.TFT_Turret_Preview_BP_C'"));
	if (tpc.Succeeded())
	{
		_previewTurretClass = tpc.Class;
	}*/
	//static ConstructorHelpers::FClassFinder<UUserWidget> DialogueUI(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/DialogueWidget.DialogueWidget_C'"));
	//if (DialogueUI.Succeeded())
	//{
	//	DialogueWidgetClass = DialogueUI.Class;
	//}
	//static ConstructorHelpers::FClassFinder<UUserWidget> DialogueUI2(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/DialogueWidget2.DialogueWidget2_C'"));
	//if (DialogueUI2.Succeeded())
	//{
	//	DialogueWidget2Class = DialogueUI2.Class; // ���� Ŭ������ DialogueWidget2Class�� ����
	//}
}

void ATFT_Player::BeginPlay()
{
	Super::BeginPlay();

	//bIsDialogueActive = false;

	if (_invenCom != nullptr)
	{
		_invenCom->_itemAddedEvent.AddUObject(this, &ATFT_Player::AddItemHendle);
		_invenCom->_GoldChangeEvnet.AddUObject(this, &ATFT_Player::UIGold);
		/*UIMANAGER->_EquipmentCloseResetEvent.AddUObject(this, &ATFT_Player::CloseResetEquipment);
		UIMANAGER->GetInvenUI()->_SlotItemEvent.AddUObject(this, &ATFT_Player::DropItemPlayer);
		UIMANAGER->GetInvenUI()->_itemSellEvent.AddUObject(this, &ATFT_Player::SellItemPlayer);
		UIMANAGER->GetInvenUI()->_itemUesEvent.AddUObject(this, &ATFT_Player::UseItemPlayer);
		UIMANAGER->GetEquipmentUI()->_ItemChangeEvent.AddUObject(this, &ATFT_Player::ChangeEquipment);
		UIMANAGER->GetEquipmentUI()->_ItemChangeEvent_stat.AddUObject(this, &ATFT_Player::UseItemPlayer_Equipment);*/
		_invenCom->_BulletEvent.AddUObject(this, &ATFT_Player::BulletHendle);
		_invenCom->_BuffGetDelegate.AddUObject(this, &ATFT_Player::AddBuffItemPlayer);
		_invenCom->_RandomBoxGetDelegate.AddUObject(this, &ATFT_Player::AddRandomBoxPlayer);
	}

	if (_shieldDashAttackSphere != nullptr)
	{
		_shieldDashAttackSphere->OnComponentBeginOverlap.AddDynamic(this, &ATFT_Player::ShieldDash_OnOverlapBegin);
	}

	_cameraZoomHandler.BindUFunction(this, FName("CameraZoom"));
	_cameraTLCom->AddInterpFloat(_cameraZoomCurve, _cameraZoomHandler);

	if (UIMANAGER->GetSkillUI())
	{
		UIMANAGER->GetSkillUI()->SetSkillSlot(WEAPON_TYPE::closeRange, 0, 2.0f); // Q 0
		UIMANAGER->GetSkillUI()->SetSkillSlot(WEAPON_TYPE::closeRange, 1, 6.0f); // E 1
		UIMANAGER->GetSkillUI()->SetSkillSlot(WEAPON_TYPE::longRange, 2, 10.0f); // Q 2
		UIMANAGER->GetSkillUI()->SetSkillSlot(WEAPON_TYPE::longRange, 3, 20.0f); // E 3

		UIMANAGER->GetSkillUI()->HiddenSkillSlot();

		if (bEquipSword == true)
		{
			UIMANAGER->GetSkillUI()->VisbleSkillSlot(WEAPON_TYPE::closeRange);
		}
		if (bEquipSword == false)
		{
			UIMANAGER->GetSkillUI()->VisbleSkillSlot(WEAPON_TYPE::longRange);
		}

	}

	if (UIMANAGER && UIMANAGER->GetRandomBoxUI())
	{
		UIMANAGER->GetRandomBoxUI()->_LeftEvent.AddUObject(this, &ATFT_Player::RandomBoxHandle);
		UIMANAGER->GetRandomBoxUI()->_RightEvent.AddUObject(this, &ATFT_Player::RandomBoxHandle);
	}

	_shield->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//_statCom->SetLevelAndInit(1);
}

void ATFT_Player::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	_statCom->SetLevelAndInit(101);

	_animInstancePlayer = Cast<UTFT_AnimInstance_Player>(GetMesh()->GetAnimInstance());
	if (_animInstancePlayer->IsValidLowLevel())
	{
		_animInstancePlayer->_dashEndDelegate.AddUObject(this, &ATFT_Player::DashEnd);
		_animInstancePlayer->_attackEndDelegate.AddUObject(this, &ATFT_Player::AttackEnd);
		_animInstancePlayer->OnMontageEnded.AddDynamic(this, &ATFT_Creature::OnAttackEnded);
		_animInstancePlayer->_shieldDashEndDelegate.AddUObject(this, &ATFT_Player::StopShieldDash);
		_animInstancePlayer->_attackHitDelegate.AddUObject(this, &ATFT_Player::AttackHit);
		_animInstancePlayer->_ShieldDashCollisionOnDelegate.AddUObject(this, &ATFT_Player::ShieldDashCollisionOn);
		_animInstancePlayer->_eSkillHitDelegate.AddUObject(this, &ATFT_Player::E_SkillHit);
		_animInstancePlayer->_fireDelegate.AddUObject(this, &ATFT_Player::Fire);
		_animInstancePlayer->_DeathDelegate.AddUObject(this, &ATFT_Player::DeathPlayer);
		//_animInstancePlayer->_stateMontageEndDelegate.AddUObject(this, &ATFT_Player::EndState);
		_animInstancePlayer->_footStepDelegate.AddUObject(this, &ATFT_Player::FootStep);
	}

	if (HpBarWidgetInstance)
	{

		UTFT_HPBarWidget* HpBar = Cast<UTFT_HPBarWidget>(HpBarWidgetInstance);
		if (HpBar)
		{
			HpBar->SetProfileImage(ProfileType::PLAYER);
			HpBar->SetHpText(_statCom->GetMaxHp());
			HpBar->SetExpText(_statCom->GetMaxExp());
			_statCom->_PlayerhpChangedDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::SetHpBarValue);
			_statCom->_CurHpText.AddUObject(HpBar, &UTFT_HPBarWidget::CurHpText);
			_statCom->_PlayerBarrierChangedDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::SetBarrierBarValue);
			_statCom->_CurBarrierText.AddUObject(HpBar, &UTFT_HPBarWidget::SetBarrierText);
			_statCom->_expChangedDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::SetExpBarValue);
			_statCom->_CurExpText.AddUObject(HpBar, &UTFT_HPBarWidget::CurExpText);
			_statCom->_levelUpDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::SetLevelText);
			_statCom->_HpUpDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::SetMaxHPText);
			_statCom->_ExpUpDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::SetExpText);
			_statCom->_maxHpChangerdDelegate.AddUObject(HpBar, &UTFT_HPBarWidget::MaxHpChangeHandle);
		}
	}

	//if (_stateCom->IsValidLowLevel())
	//{
	//	_stateCom->_stateChangeDelegate.AddUObject(this, &ATFT_Player::StateCheck);
	//}

	if (_invenCom->IsValidLowLevel())
	{
		_invenCom->_pairWeaponUIDelegate.AddUObject(this, &ATFT_Player::PairWeaponUI);
		//_invenCom->_BuffGetDelegate.AddUObject(this, )
	}
}

void ATFT_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_canMove)
	{
		RotatePlayer(DeltaTime);
	}
}

void ATFT_Player::Init()
{
}

void ATFT_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(_moveAction, ETriggerEvent::Triggered, this, &ATFT_Player::Move);

		EnhancedInputComponent->BindAction(_lookAction, ETriggerEvent::Triggered, this, &ATFT_Player::Look);

		EnhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Started, this, &ATFT_Player::JumpA);

		EnhancedInputComponent->BindAction(_doubleTapDash_W_Action, ETriggerEvent::Triggered, this, &ATFT_Player::DoubleTapDash_Front);
		EnhancedInputComponent->BindAction(_doubleTapDash_A_Action, ETriggerEvent::Triggered, this, &ATFT_Player::DoubleTapDash_Left);
		EnhancedInputComponent->BindAction(_doubleTapDash_S_Action, ETriggerEvent::Triggered, this, &ATFT_Player::DoubleTapDash_Back);
		EnhancedInputComponent->BindAction(_doubleTapDash_D_Action, ETriggerEvent::Triggered, this, &ATFT_Player::DoubleTapDash_Right);

		EnhancedInputComponent->BindAction(_attackAction, ETriggerEvent::Triggered, this, &ATFT_Player::AttackA);

		//EnhancedInputComponent->BindAction(_invenOpenAction, ETriggerEvent::Started, this, &ATFT_Player::InvenopenA);

		//EnhancedInputComponent->BindAction(_EquipmentAction, ETriggerEvent::Started, this, &ATFT_Player::EquipmentA);

		EnhancedInputComponent->BindAction(_ESkillAction, ETriggerEvent::Started, this, &ATFT_Player::E_Skill);

		EnhancedInputComponent->BindAction(_QSkillAction, ETriggerEvent::Started, this, &ATFT_Player::Q_Skill);

		//EnhancedInputComponent->BindAction(_rollingAction, ETriggerEvent::Started, this, &ATFT_Player::Rolling);

		EnhancedInputComponent->BindAction(_runningAction, ETriggerEvent::Started, this, &ATFT_Player::StartRunning);
		EnhancedInputComponent->BindAction(_runningAction, ETriggerEvent::Completed, this, &ATFT_Player::StopRunning);

		EnhancedInputComponent->BindAction(_defenseAction, ETriggerEvent::Started, this, &ATFT_Player::StartRightClick);
		// EnhancedInputComponent->BindAction(_defenseAction, ETriggerEvent::Started, this, &ATFT_Player::OnShield);
		EnhancedInputComponent->BindAction(_defenseAction, ETriggerEvent::Completed, this, &ATFT_Player::StopRightClick);
		// EnhancedInputComponent->BindAction(_defenseAction, ETriggerEvent::Completed, this, &ATFT_Player::OffShield);

		EnhancedInputComponent->BindAction(_changeWeaponAction, ETriggerEvent::Started, this, &ATFT_Player::ChangeWeapon);
		//EnhancedInputComponent->BindAction(_CloseUI, ETriggerEvent::Started, this, &ATFT_Player::CloseDialogueUI);

		EnhancedInputComponent->BindAction(_PlayMenuAction, ETriggerEvent::Started, this, &ATFT_Player::PlayMenuOpenA);
		//EnhancedInputComponent->BindAction(_Interact, ETriggerEvent::Started, this, &ATFT_Player::Interact);

		EnhancedInputComponent->BindAction(_PlayReLoadAction, ETriggerEvent::Started, this, &ATFT_Player::BulletReLoadA);
	}
}

void ATFT_Player::SetMesh(FString path)
{
	_meshCom->SetMesh(path);
}

void ATFT_Player::ChangeWeapon(const FInputActionValue& value)
{
	if (bIsZoom) return;

	if (bIsDefense) return;

	if (bIsShieldDashing) return;

	bool isPressed = value.Get<bool>();

	if (isPressed)
	{
		UIMANAGER->GetSkillUI()->HiddenSkillSlot();

		if (_invenCom->_spareWeapon != nullptr)
		{
			if (_invenCom->_spareWeapon->GetItemID() == 2)
			{
				SOUNDMANAGER->PlaySound(TEXT("P_ChangeWeapon_Rifle"), GetActorLocation());

			}
			else
			{
				if (bTurretBuildMode)
				{
					if (_turret)
					{
						_turret->Destroy();
						_turret = nullptr;
					}

					bTurretBuildMode = false;
					bBuildTurret = false;

					GetWorldTimerManager().PauseTimer(_turretTimerHandle);
				}
				SOUNDMANAGER->PlaySound(TEXT("P_ChangeWeapon_Sword"), GetActorLocation());

			}
		}

		_invenCom->ChangeWeapon();

		PairWeaponUI();
	}
}

void ATFT_Player::PlayMenuOpenA(const FInputActionValue& value)
{
	UIMANAGER->_MenuOpenEvent.Broadcast();
}

void ATFT_Player::Move(const FInputActionValue& value)
{
	if (!_canMove) return;

	if (GetCurHp() <= 0) return;

	FVector2D MovementVector = value.Get<FVector2D>();

	if (bBlockInputOnDash) return;

	if (Controller != nullptr)
	{
		_vertical = MovementVector.Y;
		_horizontal = MovementVector.X;

		if (bIsRunning)
		{
			if (_vertical < 0)
			{
				StopRunning();

				return;
			}

			if (_horizontal != 0)
			{
				AddMovementInput(GetActorForwardVector(), MovementVector.Y);

				return;
			}
		}

		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ATFT_Player::Look(const FInputActionValue& value)
{
	if (GetCurHp() <= 0) return;

	FVector2D LookAxisVector = value.Get<FVector2D>();

	if (!_canMove)
	{
		bUseControllerRotationYaw = false;
	}

	if (Controller != nullptr)
	{
		AddControllerPitchInput(-LookAxisVector.Y);
		AddControllerYawInput(LookAxisVector.X);
	}
}

void ATFT_Player::JumpA(const FInputActionValue& value)
{
	if (GetCurHp() <= 0) return;

	if (!_canMove) return;

	if (bIsRunning) return;

	if (bBlockInputOnDash) return;

	bool isPressed = value.Get<bool>();

	if (isPressed)
	{
		ACharacter::Jump();
	}
}

void ATFT_Player::AttackA(const FInputActionValue& value)
{
	if (GetCurHp() <= 0 || bIsSkillActive) return;

	bool isPressed = value.Get<bool>();

	if (bEquipSword)
	{
		if (_isAttacking == false && isPressed && _animInstancePlayer != nullptr)
		{
			_animInstancePlayer->PlayAttackMontage();
			_isAttacking = true;
			/*_canMove = false; */


			FRotator controlRotation = GetControlRotation();
			controlRotation.Pitch = 0.0f;
			controlRotation.Roll = 0.0f;
			SetActorRotation(controlRotation);

			_curAttackIndex %= 3;
			_curAttackIndex++;
			_animInstancePlayer->JumpToSection(_curAttackIndex);

			SOUNDMANAGER->PlaySound("P_Sword_Swing", GetActorLocation());
		}
	}
	else
	{
		if (_isAttacking || GetWorldTimerManager().IsTimerActive(_BullettimerHandle)) return;

		if (bTurretBuildMode && !bBuildTurret)
		{
			bBuildTurret = true;

			return;
		}

		if (_invenCom->UseBullet())
		{
			_animInstancePlayer->PlayAttackMontage();
			_isAttacking = true;
			_curAttackIndex %= 3;
			_curAttackIndex++;
			_animInstancePlayer->JumpToSection(_curAttackIndex);

			GetWorldTimerManager().SetTimer(_BullettimerHandle, this, &ATFT_Player::AttackEnd, 0.2f, false);
		}
	}
}

//void ATFT_Player::InvenopenA(const FInputActionValue& value)
//{
//	UIMANAGER->GetInvenUI()->UIGold(_invenCom->GetPlayerGold());
//
//	UIMANAGER->_invenOpenEvent.Broadcast();
//}
//
//void ATFT_Player::EquipmentA(const FInputActionValue& value)
//{
//	UIMANAGER->_EquipmentOpenEvent.Broadcast();
//}
//
//void ATFT_Player::Rolling()
//{
//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
//	AnimInstance->Montage_Play(_rollingMontage);
//}

void ATFT_Player::CameraZoom(float alpha)
{
	_springArm->TargetArmLength = FMath::Lerp(_defalutSpringArmLength, _zoomSpringArmLength, alpha);
}

void ATFT_Player::AttackEnd()
{
	_canMove = true;

	_isAttacking = false;
	GetWorldTimerManager().ClearTimer(_BullettimerHandle);
}

void ATFT_Player::E_Skill(const FInputActionValue& value)
{
	bool isPressed = value.Get<bool>();

	if (bEquipSword)
	{
		if (UIMANAGER->GetSkillUI()->GetSkillSlot(1)->bCoolDownOn == false)
		{
			if (bIsDefense) return;


			bIsSkillActive = true;
			_canMove = false;
			bIsRunning = false;
			GetCharacterMovement()->DisableMovement();

			_animInstancePlayer->PlayUpperSwingMontage();
			UIMANAGER->GetSkillUI()->RunCDT(1);

			FTimerHandle SkillTimerHandle;
			GetWorldTimerManager().SetTimer(SkillTimerHandle, FTimerDelegate::CreateLambda([this]()
				{

					bIsSkillActive = false;
					_canMove = true;
					GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
					GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
				}), 2.0f, false);
		}
	}
	else
	{
		if (UIMANAGER->GetSkillUI()->GetSkillSlot(3)->bCoolDownOn == false)
		{
			if (bTurretBuildMode)
			{
				if (_turret)
				{
					_turret->Destroy();
					_turret = nullptr;
				}

				bTurretBuildMode = false;
				bBuildTurret = false;

				GetWorldTimerManager().PauseTimer(_turretTimerHandle);
				return;
			}

			if (GetWorldTimerManager().IsTimerPaused(_turretTimerHandle))
			{
				GetWorldTimerManager().UnPauseTimer(_turretTimerHandle);
			}

			bIsSkillActive = true;

			GetWorldTimerManager().SetTimer(_turretTimerHandle, this, &ATFT_Player::SpawnTurret, 0.05f, true);

			FTimerHandle SkillEndTimerHandle;
			GetWorldTimerManager().SetTimer(SkillEndTimerHandle, FTimerDelegate::CreateLambda([this]()
				{
					bIsSkillActive = false;
				}), 2.0f, false);
		}
	}
}

void ATFT_Player::Q_Skill(const FInputActionValue& value)
{
	if (GetCurHp() <= 0 || bIsSkillActive) return;

	bool isPressed = value.Get<bool>();

	if (bEquipSword)
	{
		if (UIMANAGER->GetSkillUI()->GetSkillSlot(0)->bCoolDownOn == false)
		{
			if (bIsDefense) return;


			bIsSkillActive = true;

			_animInstancePlayer->PlayShieldDashMontage();
			UIMANAGER->GetSkillUI()->RunCDT(0);


			FTimerHandle SwordSkillTimerHandle;
			GetWorldTimerManager().SetTimer(SwordSkillTimerHandle, FTimerDelegate::CreateLambda([this]()
				{
					bIsSkillActive = false;
				}), 2.0f, false);
		}
	}
	else
	{
		if (UIMANAGER->GetSkillUI()->GetSkillSlot(2)->bCoolDownOn == false)
		{
			if (_laserClass)
			{

				bIsSkillActive = true;

				FVector start = GetActorForwardVector() + FVector(40.0f, 10.0f, 50.0f);
				FVector end = (GetControlRotation().Vector()) + start;

				FVector fireLocation = GetActorLocation() + start;
				FRotator fireRotation = GetControlRotation();

				auto raser = GetWorld()->SpawnActor<AActor>(_laserClass, FVector::ZeroVector, fireRotation);

				FName HL_Fire_Socket(TEXT("hand_l_fire_socket"));
				raser->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HL_Fire_Socket);
				raser->SetOwner(this);

				UIMANAGER->GetSkillUI()->RunCDT(2);


				FTimerHandle GunSkillTimerHandle;
				GetWorldTimerManager().SetTimer(GunSkillTimerHandle, FTimerDelegate::CreateLambda([this]()
					{
						bIsSkillActive = false;
					}), 5.0f, false);
			}
		}
	}
}

void ATFT_Player::DoubleTapDash_Front(const FInputActionValue& value)
{
	if (GetCurHp() <= 0 || bIsDashCooldown) return;

	if (bIsRunning || !_canMove) return;

	if (bCanDash && !GetCharacterMovement()->IsFalling())
	{
		bIsDashing = true;
		bFDashing = true;
		bBlockInputOnDash = true;
		bCanDash = false;
		bIsDashCooldown = true;

		FVector from = GetActorLocation();
		FVector to = GetWorld()->GetFirstPlayerController()->GetPawn()->FindComponentByClass<UCameraComponent>()->GetComponentLocation();
		to.Z = from.Z;

		FVector _dir = UKismetMathLibrary::GetDirectionUnitVector(from, to).RotateAngleAxis(180.0f, FVector(0, 0, 1));
		LaunchCharacter((_dir * dashStrength_Ground), false, false);

		FTimerHandle DashCooldownTimerHandle;
		GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, FTimerDelegate::CreateLambda([this]()
			{
				bIsDashCooldown = false;
			}), DashCooldownTime, false);

		FTimerHandle _timerHandle;
		GetWorldTimerManager().SetTimer(_timerHandle, this, &ATFT_Player::SetBlockInputOnDash_False, 0.5f, false);

		SOUNDMANAGER->PlaySound("P_Dash", from);
	}
}

void ATFT_Player::DoubleTapDash_Back(const FInputActionValue& value)
{
	if (GetCurHp() <= 0 || bIsDashCooldown) return;

	if (bIsRunning || !_canMove) return;

	if (bCanDash && !GetCharacterMovement()->IsFalling())
	{
		bIsDashing = true;
		bBDashing = true;
		bBlockInputOnDash = true;
		bCanDash = false;
		bIsDashCooldown = true;

		FVector from = GetActorLocation();
		FVector to = GetWorld()->GetFirstPlayerController()->GetPawn()->FindComponentByClass<UCameraComponent>()->GetComponentLocation();
		to.Z = from.Z;

		FVector _dir = UKismetMathLibrary::GetDirectionUnitVector(from, to);
		LaunchCharacter((_dir * dashStrength_Ground), false, false);

		FTimerHandle DashCooldownTimerHandle;
		GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, FTimerDelegate::CreateLambda([this]()
			{
				bIsDashCooldown = false;
			}), DashCooldownTime, false);

		FTimerHandle _timerHandle;
		GetWorldTimerManager().SetTimer(_timerHandle, this, &ATFT_Player::SetBlockInputOnDash_False, 0.5f, false);

		SOUNDMANAGER->PlaySound("P_Dash", from);
	}
}

void ATFT_Player::DoubleTapDash_Left(const FInputActionValue& value)
{
	if (GetCurHp() <= 0 || bIsDashCooldown) return;

	if (bIsRunning || !_canMove) return;

	if (bCanDash && !GetCharacterMovement()->IsFalling())
	{
		bIsDashing = true;
		bLDashing = true;
		bBlockInputOnDash = true;
		bCanDash = false;
		bIsDashCooldown = true;

		FVector from = GetActorLocation();
		FVector to = GetWorld()->GetFirstPlayerController()->GetPawn()->FindComponentByClass<UCameraComponent>()->GetComponentLocation();
		to.Z = from.Z;

		FVector _dir = UKismetMathLibrary::GetDirectionUnitVector(from, to).RotateAngleAxis(90.0f, FVector(0, 0, 1));
		LaunchCharacter((_dir * dashStrength_Ground), false, false);

		FTimerHandle DashCooldownTimerHandle;
		GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, FTimerDelegate::CreateLambda([this]()
			{
				bIsDashCooldown = false;
			}), DashCooldownTime, false);

		FTimerHandle _timerHandle;
		GetWorldTimerManager().SetTimer(_timerHandle, this, &ATFT_Player::SetBlockInputOnDash_False, 0.5f, false);

		SOUNDMANAGER->PlaySound("P_Dash", from);
	}
}

void ATFT_Player::DoubleTapDash_Right(const FInputActionValue& value)
{
	if (GetCurHp() <= 0 || bIsDashCooldown) return;

	if (bIsRunning || !_canMove) return;

	if (bCanDash && !GetCharacterMovement()->IsFalling())
	{
		bIsDashing = true;
		bRDashing = true;
		bBlockInputOnDash = true;
		bCanDash = false;
		bIsDashCooldown = true;

		FVector from = GetActorLocation();
		FVector to = GetWorld()->GetFirstPlayerController()->GetPawn()->FindComponentByClass<UCameraComponent>()->GetComponentLocation();
		to.Z = from.Z;

		FVector _dir = UKismetMathLibrary::GetDirectionUnitVector(from, to).RotateAngleAxis(270.0f, FVector(0, 0, 1));
		LaunchCharacter((_dir * dashStrength_Ground), false, false);

		FTimerHandle DashCooldownTimerHandle;
		GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, FTimerDelegate::CreateLambda([this]()
			{
				bIsDashCooldown = false;
			}), DashCooldownTime, false);

		FTimerHandle _timerHandle;
		GetWorldTimerManager().SetTimer(_timerHandle, this, &ATFT_Player::SetBlockInputOnDash_False, 0.5f, false);

		SOUNDMANAGER->PlaySound("P_Dash", from);
	}
}

void ATFT_Player::DashEnd()
{
	if (!bCanDash)
	{
		bCanDash = true;
		bBlockInputOnDash = false;

		GetCharacterMovement()->StopMovementImmediately();

		bFDashing = false;
		bBDashing = false;
		bLDashing = false;
		bRDashing = false;

		bIsDashing = false;

		SOUNDMANAGER->PlaySound("P_FootStep", GetActorLocation());
	}
}

void ATFT_Player::StartRunning()
{
	if (!_canMove) return;

	if (bIsDefense || bIsZoom) return;

	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = runningSpeed;

	if (_animInstancePlayer)
	{
		_animInstancePlayer->PlayRunningMontage();
	}
}

void ATFT_Player::StopRunning()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	if (_animInstancePlayer)
	{
		_animInstancePlayer->StopRunningMontage();
	}
}

void ATFT_Player::StartRightClick()
{
	if (bIsShieldDashing) return;
	if (!_canMove) return;

	GetCharacterMovement()->MaxWalkSpeed = defenseWalkSpeed;

	if (bEquipSword)
	{
		bIsDefense = true;

		if (_animInstancePlayer)
		{
			EFFECTMANAGER->Play(TEXT("ShieldOn"), 0, GetActorLocation());
			_animInstancePlayer->bIsDefensing = true;
			_animInstancePlayer->PlayDefenseMontage();
		}

		OnShield();

		SOUNDMANAGER->PlaySound(TEXT("P_Shield_On"), GetActorLocation());
	}
	else
	{
		_cameraTLCom->Play();
		bIsZoom = true;
	}
}

void ATFT_Player::StopRightClick()
{
	bIsDefense = false;
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	if (bEquipSword)
	{
		if (_animInstancePlayer)
		{
			_animInstancePlayer->bIsDefensing = false;
			_animInstancePlayer->StopDefenseMontage();
		}

		OffShield();

		SOUNDMANAGER->PlaySound(TEXT("P_Shield_Off"), GetActorLocation());
	}
	else
	{
		_cameraTLCom->Reverse();
		bIsZoom = false;
	}
}

void ATFT_Player::StopShieldDash()
{
	bIsShieldDashing = false;

	OffShield();
}

void ATFT_Player::OffShield()
{
	if (bIsShieldDashing) return;

	_shield->SetVisibility(false);
	_shield->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	_shieldDashAttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// SOUNDMANAGER->PlaySound(TEXT("P_Shield_Off"), GetActorLocation());
}

void ATFT_Player::OnShield()
{
	_shield->SetVisibility(true);
	_shield->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// SOUNDMANAGER->PlaySound(TEXT("P_Shield_On"), GetActorLocation());
}

void ATFT_Player::ShieldDashCollisionOn()
{
	_shieldDashAttackSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

float ATFT_Player::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (Damage >= 30.0f)
	{
		if (_damagedCameraShaker)
		{
			GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(_damagedCameraShaker);
		}
	}

	if (bIsDefense)
	{
		SOUNDMANAGER->PlaySound("P_Shield_Hit", GetActorLocation());
		return 0.0f;
	}

	return 0.0f;
}

//void ATFT_Player::StateCheck()
//{
//	Super::StateCheck();
//
//	if (_statCom->IsDead()) return;
//
//	auto curStates = _stateCom->GetCurStates();
//
//	if (curStates.IsEmpty()) return;
//
//	/*StopRunning();
//
//	for (auto state : curStates)
//	{
//		switch (state)
//		{
//		case StateType::Airborne:
//
//			bIsOnState = true;
//			_animInstancePlayer->PlayAirborneMontage();
//			_canMove = false;
//			_stateCom->InitState();
//			break;
//
//		case StateType::Stun:
//			bIsOnState = true;
//			_animInstancePlayer->PlayStunMontage();
//			_canMove = false;
//			_stateCom->InitState();
//			break;
//
//		default:
//			break;
//		}
//	}*/
//}
//
//void ATFT_Player::EndState()
//{
//	_canMove = true;
//	bIsOnState = false;
//}

void ATFT_Player::AttackStart()
{
	/*if (_invenCom->_currentWeapon->_Itemid == 1)
	{
		SoundManager->Play("Knight_Swing", GetActorLocation());
	}
	else if (_invenCom->_currentWeapon->_Itemid == 3)
	{
		SoundManager->Play("Hammer_Swing", GetActorLocation());
	}*/
}

void ATFT_Player::AttackHit()
{
	_skillCom->AttackHit(_statCom->GetAttackDamage(), GetController());

	/*FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	float attackRange = 100.0f;
	float attackRadius = 100.0f;

	bool bResult = GetWorld()->SweepSingleByChannel
	(
		hitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * attackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(attackRadius),
		params
	);

	FVector vec = GetActorForwardVector() * attackRange;
	FVector center = GetActorLocation() + vec;// *0.5f;
	FColor drawColor = FColor::Green;

	if (bResult && hitResult.GetActor()->IsValidLowLevel())
	{
		drawColor = FColor::Red;
		FDamageEvent damageEvent;

		float actualDamage = hitResult.GetActor()->TakeDamage(_statCom->GetAttackDamage(), damageEvent, GetController(), this);
		_hitPoint = hitResult.ImpactPoint;

		if (actualDamage > 0)
		{
			ATFT_Creature* target = Cast<ATFT_Creature>(hitResult.GetActor());
			if (target != nullptr && !target->bIsOnState)
			{
				switch (_curAttackIndex)
				{
				case 1:
					target->SetState(StateType::Airborne);
					break;
				case 2:
					target->SetState(StateType::Stun);
					break;
				default:
					break;
				}
			}
		}
	}

	DrawDebugSphere(GetWorld(), center, attackRadius, 20, drawColor, false, 2.0f);*/
}

void ATFT_Player::Q_SkillHit()
{
	_skillCom->Q_SkillHit(_statCom->GetQSkillDamage(), GetController());
	//FHitResult hitResult;
	//FCollisionQueryParams params(NAME_None, false, this);

	//float attackRange = 500.0f;
	//float attackRadius = 100.0f;

	//bool bResult = GetWorld()->SweepSingleByChannel
	//(
	//	hitResult,
	//	GetActorLocation(),
	//	GetActorLocation() + GetActorForwardVector() * attackRange,
	//	FQuat::Identity,
	//	ECollisionChannel::ECC_GameTraceChannel7,
	//	FCollisionShape::MakeSphere(attackRadius),
	//	params
	//);

	//FVector vec = GetActorForwardVector() * attackRange;
	//FVector center = GetActorLocation() + vec * 0.5f;
	//FColor drawColor = FColor::Green;

	//if (bResult && hitResult.GetActor()->IsValidLowLevel())
	//{
	//	drawColor = FColor::Red;
	//	FDamageEvent damageEvent;

	//	float actualDamage = hitResult.GetActor()->TakeDamage(30.0f, damageEvent, GetController(), this);
	//	_hitPoint = hitResult.ImpactPoint;

	//	if (actualDamage > 0)
	//	{
	//		ATFT_Creature* target = Cast<ATFT_Creature>(hitResult.GetActor());
	//		if (target != nullptr && !target->bIsOnState)
	//		{
	//			target->SetState(StateType::Stun);
	//		}
	//	}
	//}

	//DrawDebugSphere(GetWorld(), center, attackRadius, 20, drawColor, false, 0.1f);
}

void ATFT_Player::E_SkillHit()
{
	TArray<FHitResult> hitResults;
	FCollisionQueryParams params(NAME_None, false, this);

	float attackRange = 1000.0f;
	float attackRadius = 1000.0f;

	bool bResult = GetWorld()->SweepMultiByChannel(
		hitResults,
		GetActorLocation(),
		GetActorLocation() + FVector::UpVector * 0.1f,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(attackRadius),
		params
	);

	FColor drawColor = FColor::Green;


	for (const FHitResult& hitResult : hitResults)
	{
		if (hitResult.GetActor()->IsValidLowLevel())
		{
			drawColor = FColor::Red;
			FDamageEvent damageEvent;

			float actualDamage = hitResult.GetActor()->TakeDamage(30.0f, damageEvent, GetController(), this);

			/*ATFT_Monster* target = Cast<ATFT_Monster>(hitResult.GetActor());
			if (target != nullptr && !target->bIsOnState)
			{
				target->SetState(StateType::Airborne);
			}*/
		}
	}

	// ����� ��ü �׸���
	// DrawDebugSphere(GetWorld(), GetActorLocation(), attackRadius, 20, drawColor, false, 0.1f);
}

void ATFT_Player::SpawnTurret()
{
	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);

	FVector start = GetActorForwardVector();
	FVector end = (GetControlRotation().Vector()) + start;
	FVector direction = end - start;

	FVector lineStart = GetActorLocation() + start;
	FVector lineEnd = GetActorLocation() + end * 500.0f;

	bool bResult = GetWorld()->LineTraceSingleByChannel
	(
		hitResult,
		lineStart,
		lineEnd,
		ECollisionChannel::ECC_GameTraceChannel10,
		params
	);

	FColor drawColor = FColor::Green;

	auto socket = GetMesh()->GetSocketByName("turret_socket");

	if (bResult)
	{
		drawColor = FColor::Red;
		// DrawDebugLine(GetWorld(), lineStart, lineEnd, drawColor, false, 1.0f);

		if (!bTurretBuildMode)
		{
			auto previewTurret = GetWorld()->SpawnActor<ATFT_Turret>(_previewTurretClass, hitResult.ImpactPoint, FRotator::ZeroRotator);
			_turret = previewTurret;
			_turret->SetActorEnableCollision(false);
			bTurretBuildMode = true;
		}

		if (_turret)
		{
			_turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, socket->SocketName);
			_turret->SetOwner(this);
			_turret->SetActorLocation(hitResult.ImpactPoint);
		}
	}


	if (bBuildTurret && _turret)
	{
		_turret->SetActorEnableCollision(false);
		_turret->SetActorHiddenInGame(true);

		auto turret = GetWorld()->SpawnActor<ATFT_Turret>(_turretClass, _turret->GetActorLocation(), FRotator::ZeroRotator);

		turret->bIsPreview = false;

		_turret->Destroy();
		_turret = nullptr;

		bBuildTurret = false;
		bTurretBuildMode = false;

		GetWorldTimerManager().PauseTimer(_turretTimerHandle);
		UIMANAGER->GetSkillUI()->RunCDT(3);
	}

	 DrawDebugLine(GetWorld(), lineStart, lineEnd, drawColor, false, 1.0f);
}

void ATFT_Player::Fire()
{
	if (_projectileClass)
	{
		if (_invenCom->_currentWeapon != nullptr && _invenCom->_currentWeapon->GetItemID() == 2)
		{
			FName hand_l_fire_socket(TEXT("hand_l_fire_socket"));
			auto fireSocket = GetMesh()->GetSocketByName(hand_l_fire_socket);

			FVector start = fireSocket->GetSocketLocation(GetMesh());
			FVector end = (GetControlRotation().Vector()) + start;
			_projectileDir = end - start;

			FVector fireLocation = GetActorLocation() + start;
			FRotator fireRotation = GetControlRotation();

			/*auto bullet = GetWorld()->SpawnActor<ATFT_Projectile>(_projectileClass, start, fireRotation);
			bullet->FireInDirection(_projectileDir);*/

			_skillCom->Fire(start, fireRotation, _projectileDir);

			if (_fireCameraShaker)
			{
				GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(_fireCameraShaker);
			}

			SOUNDMANAGER->PlaySound(TEXT("P_Rifle_Fire"), start, fireRotation);
			EFFECTMANAGER->Play(TEXT("FireMuzzle"), 0, start);
		}
	}
}

void ATFT_Player::DeathPlayer()
{
	_animInstancePlayer->StopAllMontages(0.5f);
	UIMANAGER->DeathUIA();
}

void ATFT_Player::RotatePlayer(float DeltaTime)
{
	FRotator pRot = GetActorRotation();
	FRotator cRot = GetControlRotation();

	FRotator nRot = FMath::RInterpConstantTo(FRotator(0.0f, pRot.Yaw, 0.0f), FRotator(0.0f, cRot.Yaw, 0.0f), DeltaTime, 450.0f);
	SetActorRotation(FRotator(0.0f, nRot.Yaw, 0.0f));
}

void ATFT_Player::FootStep()
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
			SOUNDMANAGER->FadeOutSound("P_FootStep", 0.2f);
			SOUNDMANAGER->PlaySound("P_FootStep", end);
			break;
		default:
			break;
		}
	}
}

void ATFT_Player::AddItemPlayer(ATFT_Item* item)
{
	if (_invenCom != nullptr) _invenCom->AddItem(item);
}

void ATFT_Player::AddItemHendle(ATFT_Item* item, int32 index)
{
	//UIMANAGER->GetInvenUI()->AddUiItem(item, index);
}

void ATFT_Player::DropItemPlayer(ATFT_Item* item, int32 index)
{
	if (_invenCom != nullptr) _invenCom->DropItem(index);
}

void ATFT_Player::SellItemPlayer(ATFT_Item* item, int32 index)
{
	if (_invenCom != nullptr)  _invenCom->SellItem(index);
}

void ATFT_Player::UseItemPlayer(ATFT_Item* item, int32 index)
{
	if (item->GetItemType() == "Equipment")
	{
		UE_LOG(LogTemp, Log, TEXT("Equipment Item Use"));

		//UIMANAGER->GetEquipmentUI()->Set_Equipment(item);
		_invenCom->UseItem(index);
	}
	else if (item->GetItemType() == "Utility")
	{
		UE_LOG(LogTemp, Log, TEXT("Utility Item Use"));

		_invenCom->UseItem(index);
	}
}

void ATFT_Player::UseItemPlayer_Equipment(ATFT_Item* item)
{
	if (_statCom != nullptr) _statCom->AddAttackDamage(item->GetItemAttackDamage());

}

void ATFT_Player::UIGold(int32 gold)
{
	//UIMANAGER->GetInvenUI()->UIGold(gold);

}

void ATFT_Player::ChangeEquipment(ATFT_Item* item)
{
	if (_statCom != nullptr) _statCom->AddAttackDamage(-item->GetItemAttackDamage());

	_invenCom->AddItem(item);
}

void ATFT_Player::CloseResetEquipment()
{
	//UIMANAGER->GetEquipmentUI()->ResetChoice();
}

void ATFT_Player::AddBuffItemPlayer(ATFT_Item* item)
{
	if (item->GetItemRegion() == "HP")
	{
		UE_LOG(LogTemp, Warning, TEXT("HP heal~"));
		_statCom->AddCurHp(item->GetItemAttackDamage());
	}
	else if (item->GetItemRegion() == "Barrier")
	{
		UE_LOG(LogTemp, Warning, TEXT("Barrier up"));
		_statCom->AddCurBarrier(item->GetItemAttackDamage());
	}
}

void ATFT_Player::AddRandomBoxPlayer(ATFT_Item* item)
{
	UIMANAGER->_RandomBoxOpenEvent.Broadcast();


	int32 randoms = FMath::RandRange(1, 2);
	if (randoms == 1)
	{
		_statCom->AddMaxHp(50);
		UE_LOG(LogTemp, Error, TEXT("Max HP 50 PLUS !!"));
	}
	else if (randoms == 2)
	{
		_statCom->AddMaxBarrier(50);
		UE_LOG(LogTemp, Error, TEXT("Max Barrier 50 PLUS"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Not Events"));
	}
}

void ATFT_Player::BulletHendle(int32 curBullet, int32 ALLBullet)
{
	UIMANAGER->GetSkillUI()->ThisBulletText(curBullet, ALLBullet);
}

void ATFT_Player::BulletReLoadA()
{
	_invenCom->ReLoadBullet();
}

void ATFT_Player::RandomBoxHandle(int32 type, int32 value)
{
	if (type == 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Max Hp %d PLUS ~~!"), value);
		_statCom->AddMaxHp(value);
	}
	else if (type == 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Max Barrier %d PLUS ~~!"), value);
		_statCom->AddMaxBarrier(value);
	}
	else if (type == 3)
	{
		UE_LOG(LogTemp, Error, TEXT("Max Roader %d PLUS ~~!"), value);
		_invenCom->AddMaxRoadBullet(value);
	}
	else if (type == 4)
	{
		UE_LOG(LogTemp, Error, TEXT("Supplt : Ammo  %d EA PLUS ~~!"), value);

		if (bEquipSword) _invenCom->AddSuppltBullet(value);
		else _invenCom->AddPlayerBullet(value);
	}
}

void ATFT_Player::PairWeaponUI()
{
	if (_invenCom->_currentWeapon != nullptr)
	{
		UIMANAGER->GetSkillUI()->HiddenSkillSlot();

		switch (_invenCom->_currentWeapon->GetItemID())
		{
		case 1:
			bEquipSword = true;
			UIMANAGER->GetSkillUI()->VisbleSkillSlot(WEAPON_TYPE::closeRange);
			UIMANAGER->GetSkillUI()->SetClosedWeaponStackText();
			break;

		case 2:
			bEquipSword = false;
			UIMANAGER->GetSkillUI()->VisbleSkillSlot(WEAPON_TYPE::longRange);
			_invenCom->SetcurBullet();
			break;

		default:
			break;
		}
	}
}

//void ATFT_Player::StartDialogueUI()
//{
//	if (bIsDialogueActive)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Dialogue UI is already active."));
//		return;
//	}
//
//
//	if (!DialogueWidgetInstance && DialogueWidgetClass)
//	{
//		DialogueWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), DialogueWidgetClass);
//		if (DialogueWidgetInstance)
//		{
//			DialogueWidgetInstance->AddToViewport();
//			bIsDialogueActive = true;
//			UE_LOG(LogTemp, Warning, TEXT("Dialogue UI opened."));
//
//
//			GetWorld()->GetFirstPlayerController()->SetIgnoreMoveInput(true);
//		}
//	}
//}
//
//void ATFT_Player::CloseDialogueUI()
//{
//	if (bIsDialogueActive)
//	{
//		// NPC�� UI â �ݱ�
//		if (DialogueWidgetInstance)
//		{
//			//ycg - change
//			//DialogueWidgetInstance->RemoveFromViewport();
//			DialogueWidgetInstance->RemoveFromParent();
//			DialogueWidgetInstance = nullptr;
//		}
//
//		bIsDialogueActive = false;
//		GetWorld()->GetFirstPlayerController()->SetIgnoreMoveInput(false);
//
//		// �±׷� �� ����
//		for (TActorIterator<ATFT_NPC> NPCItr(GetWorld()); NPCItr; ++NPCItr)
//		{
//			ATFT_NPC* NPC = *NPCItr;
//			if (NPC && NPC->InteractionBox->IsOverlappingActor(this)) // �÷��̾�� ��ģ NPC ã��
//			{
//				NPC->HideNPC(); // NPC �����
//				OpenTaggedDoor(TEXT("Door1")); // �±׷� ù ��° �� ����
//				break;
//			}
//		}
//
//		// NPC2�� UI â �ݱ�
//		if (DialogueWidget2Instance)
//		{
//			//ycg - change
//			//DialogueWidget2Instance->RemoveFromViewport();
//			DialogueWidget2Instance->RemoveFromParent();
//			DialogueWidget2Instance = nullptr;
//		}
//
//		// NPC2 ����� �� �±׷� �� ��° �� ����
//		for (TActorIterator<ATFT_NPC2> NPC2Itr(GetWorld()); NPC2Itr; ++NPC2Itr)
//		{
//			ATFT_NPC2* NPC2 = *NPC2Itr;
//			if (NPC2 && NPC2->InteractionBox->IsOverlappingActor(this)) // �÷��̾�� ��ģ NPC2 ã��
//			{
//				NPC2->HideNPC(); // NPC2 �����
//				OpenTaggedDoor(TEXT("Door2")); // �±׷� �� ��° �� ����
//				break;
//			}
//		}
//	}
//}
//
//void ATFT_Player::StartDialogueUI2()
//{
//	if (bIsDialogueActive)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Dialogue UI2 is already active."));
//		return;
//	}
//
//	if (!DialogueWidget2Instance && DialogueWidget2Class)
//	{
//		DialogueWidget2Instance = CreateWidget<UUserWidget>(GetWorld(), DialogueWidget2Class);
//		if (DialogueWidget2Instance)
//		{
//			DialogueWidget2Instance->AddToViewport();
//			bIsDialogueActive = true;
//			UE_LOG(LogTemp, Warning, TEXT("Dialogue UI2 opened."));
//
//			GetWorld()->GetFirstPlayerController()->SetIgnoreMoveInput(true);
//		}
//	}
//}
//
//void ATFT_Player::OpenTaggedDoor(FName DoorTag)
//{
//	for (TActorIterator<ATFT_Door> DoorItr(GetWorld()); DoorItr; ++DoorItr)
//	{
//		ATFT_Door* Door = *DoorItr;
//		if (Door && Door->ActorHasTag(DoorTag)) // �±װ� ��ġ�ϴ� �� ã��
//		{
//			Door->OpenDoor(); // �� ����
//
//			// 3�� �� �� �ݱ�
//			FTimerHandle TimerHandle;
//			GetWorldTimerManager().SetTimer(TimerHandle, [Door]()
//				{
//					Door->CloseDoor();
//				}, 3.0f, false);
//
//			break; // ù ��° ��ġ�ϴ� ���� ó��
//		}
//	}
//}
//
void ATFT_Player::Interact()
{
	//TArray<AActor*> OverlappingActors;
	//GetOverlappingActors(OverlappingActors, ATFT_Button::StaticClass());

	//for (AActor* Actor : OverlappingActors)
	//{
	//	ATFT_Button* Button = Cast<ATFT_Button>(Actor);
	//	if (Button && Button->bPlayerNearby) // ��ư ��ó�� ���� ���� ���
	//	{
	//		Button->ToggleButton();
	//		break;
	//	}
	//}
}



void ATFT_Player::ShieldDash_OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto enemy = Cast<ATFT_Monster>(OtherActor);

	if (enemy != nullptr)
	{
		Q_SkillHit();

		_shieldDashAttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//_animInstancePlayer->StopShiedlDashMontage();
	}
}