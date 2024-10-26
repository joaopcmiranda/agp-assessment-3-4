// Fill out your copyright notice in the Description page of Project Settings.


#include "FixedBeingsManagerEditorSubsystem.h"
#include "FixedBeing.h"

void UFixedBeingsManagerEditorSubsystem::Deinitialize()
{
	Super::Deinitialize();

	ClearAll();
}


void UFixedBeingsManagerEditorSubsystem::ClearAll()
{
	for (auto [_, Actor] : SpawnedBeings)
	{
		if (Actor && IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedBeings.Empty();
}


