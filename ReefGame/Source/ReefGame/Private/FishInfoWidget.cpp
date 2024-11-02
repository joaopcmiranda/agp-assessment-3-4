// Fill out your copyright notice in the Description page of Project Settings.


#include "FishInfoWidget.h"

void UFishInfoWidget::SetFishType(EFishType FishType)
{
	CurrentFishType = FishType;
	UpdateFishInfoUI();
}

