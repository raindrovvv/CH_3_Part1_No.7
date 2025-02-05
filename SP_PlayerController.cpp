// Fill out your copyright notice in the Description page of Project Settings.
#include "SP_PlayerController.h"
#include "EnhancedInputSubsystems.h"

ASP_PlayerController::ASP_PlayerController()
:	InputMappingContext(nullptr),
	MoveAction(nullptr),
	MoveForwardAction(nullptr),
	MoveRightAction(nullptr),
	LookAction(nullptr),
	JumpAction(nullptr),
	MoveUpAction(nullptr),
	RotateRollAction(nullptr),
	FlightHoldAction(nullptr)
{
}

void ASP_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem=
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
