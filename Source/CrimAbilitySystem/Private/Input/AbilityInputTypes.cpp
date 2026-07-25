// Copyright Soccertitan 2026


#include "Input/AbilityInputTypes.h"

#include "Abilities/GameplayAbility.h"


bool FAbilityInputParams::IsValid() const
{
	return Slot.IsValid() && AbilityClass;
}

#if WITH_EDITOR
void FAbilityInputParams::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s : %s"), *Slot.GetString(), *GetNameSafe(AbilityClass));
}
#endif
