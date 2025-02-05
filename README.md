## 필수 과제 (기본 요구 사항) 🐣

### **필수 과제 1번 - C++ Pawn 클래스와 충돌 컴포넌트 구성**

- **Pawn 클래스 생성**
    - 충돌 컴포넌트를 루트로 설정합니다 (Capsule/Box/Sphere 중 택 1).
    - SkeletalMeshComponent, SpringArm, CameraComponent를 Attach하여 3인칭 시점을 구성합니다.
    - `GameMode`에서 `DefaultPawnClass`를 이 Pawn 클래스로 지정합니다.
- **Physics 설정**
    - Pawn 이동은 물리 시뮬레이션이 아닌 **직접** 코드로 제어해야 합니다.
    - 따라서 **루트 충돌 컴포넌트 및 SkeletalMeshComponent 모두 Simulate Physics = false로 설정해야 합니다.**

### **필수 과제 2번 - Enhanced Input 매핑 & 바인딩**

- **Input 액션 생성**
    - MoveForward/MoveRight (WASD용)
    - Look (마우스 이동용)
    - 위 액션들을 Input Mapping Context (IMC)에 매핑합니다.
- **입력 바인딩**
    - `SetupPlayerInputComponent()`에서 각각의 액션에 함수를 바인딩합니다.
        - 예: `Move`(W/S/A/D), `Look`(마우스 이동) 등
- **이동/회전 로직 구현**
    - **Pawn 내부에서** `AddActorLocalOffset()`, `AddActorLocalRotation()`, `SetActorLocation()`등을 사용합니다.
    - 이동 방향은 Pawn의 Forward/Right 벡터에 따라 결정됩니다.
    - 마우스 입력으로 Yaw와 Pitch를 직접 계산하여 Pawn의 회전을 구현합니다.
        - `AddControllerYawInput()` 또는 `AddControllerPitchInput()` 같은 기본 제공 함수는 사용하지 않고, 직접 로직을 작성해야 합니다.
    - **중력/낙하 등의 물리 효과 없이** 평면 상에서의 이동 및 회전만 처리합니다.

## 도전 과제 (선택 요구 사항) 🦅

### **도전 과제 1번 - 6자유도 (6 DOF) 드론/비행체 구현 (난이도 상)**

- **축 분할 액션 구현**
    - **이동**
        - 전/후 (W/S) - X축 이동
        - 좌/우 (A/D) - Y축 이동
        - 상/하 (Space/Shift) - Z축 이동
    - **회전**
        - Yaw - 좌우 회전, 마우스 이동 (Z축 회전)
        - Pitch - 상하 회전, 마우스 이동 (Y축 회전)
        - Roll - 기울기 회전, 마우스 휠 (X축 회전)
- **Orientation 기반 이동 구현**
    - 단순 ‘평면 이동’이 아니라, *Pitch*나 *Roll* 각도를 기울여 전진·선회하는 식의 비행체 움직임을 구현합니다.
    - AddActorOffset/Rotation 호출 전에 **현재 회전 상태** (방향·기울기)에 따라 입력 벡터를 변환하여 적용합니다.

### **도전 과제 2번 - 중력 및 낙하 구현 (난이도 최상)**

- **인공 중력 구현**
    - **Physics 시뮬레이션 대신 Tick 로직**을 통해 인공 중력을 구현합니다.
    - 엔진 내장 `CharacterMovementComponent` 없이, **Pawn 코드**에서 매 프레임 중력 가속도를 직접 계산해야 합니다. (낙하 속도는 적절한 중력 상수 (예: -980 cm/s²) 기반으로 구현합니다.)
    - 바닥 충돌 판정을 해야 합니다. (Sweep/Trace를 하거나, `AddActorOffset` 충돌 결과 등을 활용해 지면을 감지합니다.)
    - 착지 시 낙하 중단 또는 Z 속도를 0으로 초기화해야 합니다.
- **에어컨트롤 구현 (공중 WASD 제어)**
    - 공중에서는 지상에서의 이동보다 **느리거나 제한적**이어야 합니다 (가령 30~50% 정도).
    - 낙하와 착지가 시각적으로 자연스럽게 처리될 수 있도록, 속도/가속도 변수를 써서 이동 로직을 좀 더 자연스럽게 (=지상/공중) 구분하면 좋습니다.
