// Copyright Soccertitan 2026


#include "Input/AbilityInputBlueprintFunctionLibrary.h"

FAbilityInputSlot UAbilityInputBlueprintFunctionLibrary::MakeAbilityInputSlotFromTag(const FGameplayTag InputTag)
{
	return FAbilityInputSlot(InputTag);
}

FAbilityInputSlot UAbilityInputBlueprintFunctionLibrary::MakeAbilityInputSlotFromIndex(const int32 SlotIndex)
{
	return FAbilityInputSlot(SlotIndex);
}

bool UAbilityInputBlueprintFunctionLibrary::IsAbilityInputSlotValid(const FAbilityInputSlot& AbilityInputSlot)
{
	return AbilityInputSlot.IsValid();
}

FString UAbilityInputBlueprintFunctionLibrary::GetStringFromAbilityInputSlot(const FAbilityInputSlot& AbilityInputSlot)
{
	return AbilityInputSlot.GetString();
}
