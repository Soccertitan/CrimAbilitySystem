// Copyright Soccertitan 2026


#include "Input/AbilityInputTypes.h"
#include "Input/AbilityInput.h"


bool FAbilityInputParams::IsValid() const
{
	return Slot.IsValid() && Ability;
}

#if WITH_EDITOR
void FAbilityInputParams::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s : %s"), *Slot.GetString(), *GetNameSafe(Ability));
}
#endif
