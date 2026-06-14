// Copyright Soccertitan 2026


#include "Input\AbilityInputSlot.h"


bool FAbilityInputSlot::IsValid() const
{
	return InputTag.IsValid() || SlotIndex > INDEX_NONE;
}

FString FAbilityInputSlot::GetString() const
{
	if (InputTag.IsValid())
	{
		return InputTag.ToString();
	}
	
	return FString::Printf(TEXT("%d"), SlotIndex);
}

void FAbilityInputSlot::PostSerialize(const FArchive& Ar)
{
	if (InputTag.IsValid())
	{
		SlotIndex = INDEX_NONE;
	}
	bCanChangeSlotIndex = !InputTag.IsValid();
}
