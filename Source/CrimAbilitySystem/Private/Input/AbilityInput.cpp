// Copyright Soccertitan 2026


#include "Input/AbilityInput.h"

#include "Ability/DeathGameplayAbility.h"
#include "Misc/DataValidation.h"


UAbilityInput::UAbilityInput()
{
}

EDataValidationResult UAbilityInput::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	
	if (GameplayAbility.IsNull())
	{
		Context.AddError(FText::FromString("GameplayAbility is NULL."));
		Result = EDataValidationResult::Invalid;
	}
	return Result;
}
