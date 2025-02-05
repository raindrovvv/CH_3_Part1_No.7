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
	PrimaryActorTick.bCanEverTick = true;

	// 충돌 컴포넌트 (루트)
	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	CapsuleCollision->InitCapsuleSize(35.f, 90.f);
	CapsuleCollision->SetSimulatePhysics(false); // 물리 시뮬레이션 비활성화
	RootComponent = CapsuleCollision;

	// 스켈레탈 메쉬 컴포넌트
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(CapsuleCollision);
	SkeletalMesh->SetSimulatePhysics(false);

	// 스프링 암 컴포넌트
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CapsuleCollision);
	SpringArm->TargetArmLength = 300.0f;
	
	// Pawn회전과 독립적으로 동작, 왜? 이동 방향을 카메라의 Forward/Right 벡터를 기준으로 계산하여 회전해야 하기 때문에.
	SpringArm->bUsePawnControlRotation = false; 

	// 카메라 컴포넌트
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // 카메라는 스프링 암의 회전만 따르도록 설정
}

void ASP_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ApplyGravity(DeltaTime);
	CheckGround();
	ApplyTiltEffect(DeltaTime);
}

// 입력 바인딩
void ASP_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASP_PlayerController* PlayerController = Cast<ASP_PlayerController>(GetController()))
		{
			if (PlayerController->MoveForwardAction)
			EnhancedInput->BindAction(PlayerController->MoveForwardAction, ETriggerEvent::Triggered, this, &ASP_Pawn::MoveForward);

			if (PlayerController->MoveRightAction)
			EnhancedInput->BindAction(PlayerController->MoveRightAction, ETriggerEvent::Triggered, this, &ASP_Pawn::MoveRight);

			if (PlayerController->LookAction)
			EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &ASP_Pawn::Look);

			if (PlayerController->MoveUpAction)
			EnhancedInput->BindAction(PlayerController->MoveUpAction, ETriggerEvent::Triggered, this, &ASP_Pawn::MoveUp);

			if (PlayerController->RotateRollAction)
			EnhancedInput->BindAction(PlayerController->RotateRollAction, ETriggerEvent::Triggered, this, &ASP_Pawn::RotateRoll);

			if (PlayerController->FlightHoldAction)
			EnhancedInput->BindAction(PlayerController->FlightHoldAction, ETriggerEvent::Triggered, this, &ASP_Pawn::ToggleFlightHold);
		}
	}
}

void ASP_Pawn::Move(const FVector& Direction, float AxisValue)
{
	if (FMath::IsNearlyZero(AxisValue))
	{
		Velocity = FMath::VInterpTo(Velocity, FVector::ZeroVector, GetWorld()->GetDeltaSeconds(), 2.0f);
		return;
	}

	float SpeedMultiplier = (bIsGrounded && !bIsFlightHold) ? 1.0f : AirControlFactor;
	FVector TargetVelocity = Direction * (AxisValue * MoveSpeed * SpeedMultiplier);

	// 부드러운 속도 보간
	Velocity = FMath::VInterpTo(Velocity, TargetVelocity, GetWorld()->GetDeltaSeconds(), 6.0f);
}

void ASP_Pawn::MoveForward(const FInputActionValue& Value)
{
	Move(GetActorForwardVector(), Value.Get<float>());
}

void ASP_Pawn::MoveRight(const FInputActionValue& Value)
{
	Move(GetActorRightVector(), Value.Get<float>());
}

void ASP_Pawn::MoveUp(const FInputActionValue& Value)
{
	if (bIsFlightHold) return; // 호버링 상태에서는 수직 이동 입력을 무시하여 높이 고정
	Move(FVector::UpVector, Value.Get<float>());
}

void ASP_Pawn::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Yaw 회전 (좌우 회전)
	AddActorLocalRotation(FRotator(0.0f, LookInput.X * RotationSpeed * DeltaTime, 0.0f));

	// Pitch 회전 (카메라 상하 회전)
	FRotator NewSpringArmRot = SpringArm->GetRelativeRotation();
	NewSpringArmRot.Pitch = FMath::ClampAngle(NewSpringArmRot.Pitch + LookInput.Y * RotationSpeed * DeltaTime, -60.0f, 60.0f);
	SpringArm->SetRelativeRotation(NewSpringArmRot);
}

void ASP_Pawn::RotateRoll(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	
	if (FMath::IsNearlyZero(AxisValue)) return;
	
	FRotator CurrentRotation  = GetActorRotation();
	CurrentRotation .Roll += AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();

	// Roll 보간 적용
	SetActorRotation(CurrentRotation);
}

// ============ //
// 중력 기능 추가 //
// ============ //
void ASP_Pawn::ApplyGravity(float DeltaTime)
{
	if (bIsFlightHold)
	{
		// 비행 홀드 시 중력 제거, BUT 이동 가능하도록 속도 유지
		Velocity.Z = FMath::FInterpTo(Velocity.Z, 0.0f, DeltaTime, 2.0f);
		//return; : 활성화 하면 호버링 상태로 고정돼버림
	}
    
	if (!bIsGrounded)
	{
		Velocity.Z += Gravity * DeltaTime;
	}
	FVector NewLocation = GetActorLocation() + Velocity * DeltaTime;

	// 호버링 상태이면 수직 위치를 기록된 HoverAltitude로 고정
	if (bIsFlightHold)
	{
		NewLocation.Z = HoverAltitude;
	}
	FHitResult Hit;
	SetActorLocation(NewLocation, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		bIsGrounded = true;
		Velocity.Z = 0;
	}
}

// 바닥 감지 함수
void ASP_Pawn::CheckGround()
{
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0, 0, -GroundCheckDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		if (!bIsGrounded)
		{
			Velocity.Z = FMath::FInterpTo(Velocity.Z, 0.0f, GetWorld()->GetDeltaSeconds(), 10.0f);
		}
		bIsGrounded = true;
	}
	else
	{
		bIsGrounded = false;
	}
}

// 틸트 기능!!
void ASP_Pawn::ApplyTiltEffect(float DeltaTime)
{
	FRotator CurrentRotation = GetActorRotation();
	const float MaxTiltAngle = 15.0f; 
	const float TiltSpeed = 5.0f; // 틸트 반응 속도
	const float RecoverySpeed = 5.0f; // 회복 속도
	const float SineFactor = 0.5f; // 사인 그래프를 조절하는 팩터

	// 비행 홀드 상태면 기울기를 원래 상태로 복구
	if (bIsFlightHold)
	{
		FRotator TargetRotation = CurrentRotation;
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;

		// 사인 그래프 보간을 적용하여 부드럽게 복구
		float Alpha = FMath::Sin(RecoverySpeed * DeltaTime);  
		SetActorRotation(FMath::Lerp(CurrentRotation, TargetRotation, Alpha));
		return;
	}

	// 이동 방향 벡터 정규화
	FVector LocalVelocity = GetActorRotation().UnrotateVector(Velocity).GetSafeNormal();

	// 목표 틸트 각도 계산 (속도가 작을수록 기울기도 작아지도록 보정)
	float TargetPitch = -LocalVelocity.X * MaxTiltAngle;
	float TargetRoll = LocalVelocity.Y * MaxTiltAngle;

	// 이동이 멈추면 원래 자세로 복귀
	if (Velocity.IsNearlyZero())
	{
		TargetPitch = 0.0f;
		TargetRoll = 0.0f;
	}
	
	float Alpha = FMath::Sin(TiltSpeed * DeltaTime * SineFactor);  

	FRotator TargetRotation = CurrentRotation;
	TargetRotation.Pitch = FMath::Lerp(CurrentRotation.Pitch, TargetPitch, Alpha);
	TargetRotation.Roll = FMath::Lerp(CurrentRotation.Roll, TargetRoll, Alpha);

	SetActorRotation(TargetRotation);
}

// 호버링(비행 홀드) 기능
void ASP_Pawn::ToggleFlightHold(const FInputActionValue& Value)
{
	bIsFlightHold = !bIsFlightHold;
	// 호버링 전환 시 수직 속도를 제거, 현재 높이를 기록하여 고정
	if (bIsFlightHold)
	{
		HoverAltitude = GetActorLocation().Z;
		Velocity.Z = 0.0f;
	}
}