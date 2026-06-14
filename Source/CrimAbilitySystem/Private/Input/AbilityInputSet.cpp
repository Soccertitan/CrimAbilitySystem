// Copyright Soccertitan 2026


#include "Input/AbilityInputSet.h"
#include "Input/AbilityInput.h"

void FAbilityInputItem::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s : %s"), *Slot.GetString(), *GetNameSafe(AbilityInput));
}
