// Fill out your copyright notice in the Description page of Project Settings.

#include "SP_Pawn.h"
#include "SP_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ASP_Pawn::ASP_Pawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// 충돌 컴포넌트 (루트)
	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	CapsuleCollision->InitCapsuleSize(35.f, 90.f);
	CapsuleCollision->SetSimulatePhysics(false); // 물리 시뮬레이션 비활성화
	RootComponent = CapsuleCollision;

	// 스켈레탈 메쉬 컴포넌트
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(CapsuleCollision);
	SkeletalMesh->SetSimulatePhysics(false); // 물리 시뮬레이션 비활성화

	// 스프링 암 컴포넌트
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleCollision);
	SpringArm->TargetArmLength = 300.0f; // 스프링 암 길이 설정
	
	// Pawn회전과 독립적으로 동작, 왜? 이동 방향을 카메라의 Forward/Right 벡터를 기준으로 계산하여 회전해야 하기 때문에.
	SpringArm->bUsePawnControlRotation = false; 

	// 카메라 컴포넌트
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // 카메라는 스프링 암의 회전만 따르도록 설정
}


// Called to bind functionality to input
void ASP_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASP_PlayerController* PlayerController = Cast<ASP_PlayerController>(GetController()))
		{
			if (PlayerController->MoveForwardAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveForwardAction, ETriggerEvent::Triggered, this, &ASP_Pawn::MoveForward);
			}
			if (PlayerController->MoveRightAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveRightAction, ETriggerEvent::Triggered, this, &ASP_Pawn::MoveRight);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &ASP_Pawn::Look);
			}
		}
	}
}

void ASP_Pawn::MoveForward(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue)) return;
	
	// 카메라 회전에서 Yaw만 사용하여 이동 방향 결정
	FRotator CamRot = Camera->GetComponentRotation();
	CamRot.Pitch = 0.0f;
	CamRot.Roll = 0.0f;
	FVector Direction = FRotationMatrix(CamRot).GetUnitAxis(EAxis::X);

	FVector NewLocation = GetActorLocation() + (Direction * AxisValue * MoveSpeed * GetWorld()->GetDeltaSeconds());
	SetActorLocation(NewLocation);
}

void ASP_Pawn::MoveRight(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	
	if (FMath::IsNearlyZero(AxisValue)) return;

	FRotator CamRot = Camera->GetComponentRotation();
	CamRot.Pitch = 0.0f;
	CamRot.Roll = 0.0f;
	FVector Direction = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
	
	FVector NewLocation = GetActorLocation() + (Direction * AxisValue * MoveSpeed * GetWorld()->GetDeltaSeconds());
	SetActorLocation(NewLocation);
}

void ASP_Pawn::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	// Yaw 회전 (좌우 회전)
	AddActorLocalRotation(FRotator(0.0f, LookInput.X * RotationSpeed * DeltaTime, 0.0f));

	// Pitch 회전 (상하 회전)
	FRotator NewSpringArmRot = SpringArm->GetRelativeRotation();
	NewSpringArmRot.Pitch = FMath::ClampAngle(NewSpringArmRot.Pitch + LookInput.Y * RotationSpeed * DeltaTime, -60.0f, 60.0f);
	SpringArm->SetRelativeRotation(NewSpringArmRot);
}