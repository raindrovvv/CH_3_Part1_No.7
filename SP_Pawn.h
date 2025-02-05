// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SP_Pawn.generated.h"

UCLASS()
class START_API ASP_Pawn : public APawn
{
	GENERATED_BODY()

public:
	ASP_Pawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void Move(const FVector& Direction, float AxisValue);

protected:
	// 루트 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* CapsuleCollision;
	
	// 스켈레탈 메쉬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalMesh;

	// 스프링 암 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArm;

	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* Camera;

private:
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void MoveUp(const FInputActionValue& Value);  // 상,하 이동 (드론)
	void RotateRoll(const FInputActionValue& Value); // Roll 회전 (드론)
	void ToggleFlightHold(const FInputActionValue& Value);

	// 이동 속도 및 중력
	const float MoveSpeed = 900.0f;
	const float RotationSpeed = 90.0f;
	const float Gravity = -500.0f;  // 중력 값
	const float Power = 50.0f; // 상승력 (space Bar)
	const float AirControlFactor = 0.8f; // 공중 이동 속도 감소 비율
	const float GroundCheckDistance = 10.0f; // 바닥 감지 거리
	float HoverAltitude = 0.0f;
	float YawInput = 0.0f; // Yaw 입력값

	// 중력 및 낙하 관련 함수
	void ApplyGravity(float DeltaTime);
	void CheckGround();
	void ApplyTiltEffect(float DeltaTime);
	
	// 상태 변수
	FVector Velocity = FVector::ZeroVector;  // 현재 속도 벡터
	bool bIsGrounded = false;  // 바닥 감지 여부
	bool bIsFlightHold = false; // 비행 홀드 (CTRL 키)

};
