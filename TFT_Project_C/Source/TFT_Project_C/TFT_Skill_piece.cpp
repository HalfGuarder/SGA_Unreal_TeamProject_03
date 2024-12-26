// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_Skill_piece.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Slate\SlateBrushAsset.h"

#include "Kismet/KismetMathLibrary.h"

void UTFT_Skill_piece::NativeConstruct()
{
	Super::NativeConstruct();

	//// rush
	//_brushCloseQ_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/Rush_icon.Rush_icon'"));
	//_brushCloseQ_Black_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/Rush_Black_icon.Rush_Black_icon'"));
	//// air bon
	//_brushCloseE_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/Airbon_icon.Airbon_icon'"));
	//_brushCloseE_Black_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/Airbon_Black_icon.Airbon_Black_icon'"));
	//// laser
	//_brushLongQ_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/laser_icon.laser_icon'"));
	//_brushLongQ_Black_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/laser_Black_icon.laser_Black_icon'"));
	//// turret
	//_brushLongE_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/turret_icon.turret_icon'"));
	//_brushLongE_Black_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/turret_Black_icon.turret_Black_icon'"));

	//_nullBrush_Resource = LoadObject<UTexture2D>(nullptr, TEXT("/Script/Engine.Texture2D'/Game/Blueprints/Widget/Widget_textrue/invisible.invisible'"));

	_brushCloseQ.SetResourceObject(_brushCloseQ_Resource);
	_brushCloseQ_Black.SetResourceObject(_brushCloseQ_Black_Resource);
	_brushCloseE.SetResourceObject(_brushCloseE_Resource);
	_brushCloseE_Black.SetResourceObject(_brushCloseE_Black_Resource);
	_brushLongQ.SetResourceObject(_brushLongQ_Resource);
	_brushLongQ_Black.SetResourceObject(_brushLongQ_Black_Resource);
	_brushLongE.SetResourceObject(_brushLongE_Resource);
	_brushLongE_Black.SetResourceObject(_brushLongE_Black_Resource);

	_nullBrush.SetResourceObject(_nullBrush_Resource);

	//SkillSlotImg->SetBrush(_nullBrush);
}

void UTFT_Skill_piece::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (_runTime >= _coolDownTime)
	{
		_CountTime = _coolDownTime;
		SkillCountText->SetText(_nullText);

		_runTime = 0.0f;
		bCoolDownOn = false;

		return;
	}

	if (bCoolDownOn)
	{
		_runTime += InDeltaTime;
		_CountTime -= InDeltaTime;

		float pbPercent = 1.0f - UKismetMathLibrary::NormalizeToRange(_runTime, 0.0f, _coolDownTime);


		if (changedText == false) SetCountText(_CountTime);

		SetSlotPBPercent(pbPercent);
	}
}

void UTFT_Skill_piece::SetSlotNumText(FString text)//
{

	SkillSlotNumText->SetText(FText::FromString(*text));
}

void UTFT_Skill_piece::SetSlotImg(WEAPON_TYPE itemtype, int32 socket)
{
	switch (itemtype)
	{
	case closeRange:
	{
		if (changedText)
		{
			if (socket == 0)
			{
				FProgressBarStyle NewStyle = SkillSlotPB->GetWidgetStyle();
				NewStyle.BackgroundImage = _brushCloseQ;
				NewStyle.FillImage = _brushCloseQ_Black;
				SkillSlotPB->SetWidgetStyle(NewStyle); // ������ ��Ÿ���� �ٽ� ����
			}
			else if (socket == 1)
			{
				FProgressBarStyle NewStyle = SkillSlotPB->GetWidgetStyle();
				NewStyle.BackgroundImage = _brushCloseE;
				NewStyle.FillImage = _brushCloseE_Black;
				SkillSlotPB->SetWidgetStyle(NewStyle);
			}
		}
	}
	break;
	case longRange:
	{
		if (socket == 2)
		{
			FProgressBarStyle NewStyle = SkillSlotPB->GetWidgetStyle();
			NewStyle.BackgroundImage = _brushLongQ;
			NewStyle.FillImage = _brushLongQ_Black;
			SkillSlotPB->SetWidgetStyle(NewStyle);
		}
		else if (socket == 3)
		{
			FProgressBarStyle NewStyle = SkillSlotPB->GetWidgetStyle();
			NewStyle.BackgroundImage = _brushLongE;
			NewStyle.FillImage = _brushLongE_Black;
			SkillSlotPB->SetWidgetStyle(NewStyle);
		}
	}
	break;
	default:
		break;
	}
}

void UTFT_Skill_piece::HeddenSlotImg()
{
	FProgressBarStyle NewStyle = SkillSlotPB->GetWidgetStyle();
	NewStyle.BackgroundImage = _nullBrush;
	NewStyle.FillImage = _nullBrush;
	SkillSlotPB->SetWidgetStyle(NewStyle);

	changedText = true;
	SkillCountText->SetText(_nullText);
}

void UTFT_Skill_piece::SetSlotPBPercent(float percent)
{
	SkillSlotPB->SetPercent(percent);
}

void UTFT_Skill_piece::SetCDT(float CDT)
{
	_coolDownTime = CDT;
	_CountTime = CDT;
}

void UTFT_Skill_piece::SetCountText(float count)
{
	FString countstring = FString::Printf(TEXT("%.1f"), count);
	FText countText = FText::FromString(countstring);


	SkillCountText->SetText(countText);
}