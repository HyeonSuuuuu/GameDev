https://grandiose-delphinium-56f.notion.site/2649f08c004a81e49fa9f14f5815fdd5?source=copy_link

\# 🤖 AI Game Server Setup Guide (llama.cpp)

이 가이드는 게임 서버 졸업 작품에서 LLM 추론 기능을 통합하기 위해 llama.cpp를 빌드하고 환경을 설정하는 전체 과정을 다룹니다.

---

## ⚡ Quick Start

### 1. 필수 종속성 다운로드 (Prerequisites)
프로젝트 구동 및 하드웨어 가속을 위해 아래 SDK들을 반드시 먼저 설치해야 합니다.

\* **NVIDIA CUDA Toolkit**: NVIDIA GPU 가속을 위해 필수입니다.
  * \[다운로드 링크\](https://developer.nvidia.com/cuda-downloads)
\* **Vulkan SDK**: 다양한 하드웨어 환경(AMD, Intel 등)에서의 가속을 지원합니다.
  * \[다운로드 링크\](https://vulkan.lunarg.com/sdk/home)
\* **Visual Studio 2022**: 빌드 시스템 생성 및 C++ 컴파일을 위해 필요합니다.
  * \[다운로드 링크\](https://visualstudio.microsoft.com/ko/vs/)
  * \*본 프로젝트는 Visual Studio 2022에 내장된 CMake를 사용하여 빌드되었습니다.\*

### 2. 소스 코드 및 바이너리 준비
공식 레포지토리에서 최신 소스를 가져오거나 빌드된 파일을 준비합니다.

\* **llama.cpp Releases**: \[GitHub Releases\](https://github.com/ggml-org/llama.cpp/releases)에서 최신 버전의 Source code (.zip)를 다운로드합니다.
\* 압축을 푼 폴더로 터미널(또는 Developer Command Prompt)을 이동시킵니다.

---

## 🛠️ 3. CMake 빌드 및 세팅 (Build from Source)

Visual Studio로 빌드하는 경우 반드시 **Developer Command Prompt for VS 2022**를 실행하여 경로 인식을 정상적으로 처리해야 합니다.

### Step 1: 빌드 폴더 생성
mkdir build
cd build

### Step 2: 환경별 빌드 옵션 선택
사용하려는 가속기에 맞춰 CMake 설정을 진행합니다.

\* **CUDA (NVIDIA GPU 전용)**
  cmake .. -DGGML_CUDA=ON

\* **Vulkan (범용 GPU 가속)**
  cmake .. -DGGML_VULKAN=ON

\* **공통 설정 (추천)**
  시스템 환경을 판단하여 알맞은 가속기로 자동 설정해 줍니다.
  cmake .. -B build -G "Visual Studio 17 2022" -DGGML_BACKEND_DL=ON -DGGML_CUDA=ON -DGGML_VULKAN=ON -DGGML_NATIVE=OFF

### Step 3: 프로젝트 빌드
본인의 CPU 스레드 수에 맞춰 -j 옵션을 조정하세요. (예: 12스레드 사용 시 -j 12)

\# Debug 빌드
cmake --build build --config Debug -j 12

\# Release 빌드
cmake --build build --config Release -j 12

---

## 📦 4. DLL 파일 배치

빌드가 완료되면 build/bin/Release 또는 build/bin/Debug 폴더 내에 생성된 다음 파일들을 **게임 서버 실행 파일(.exe)** 경로로 복사합니다.

\*\*복사 필수 DLL 목록:\*\*
\* llama.dll
\* ggml.dll
\* ggml-base.dll
\* ggml-cpu.dll
\* ggml-cuda.dll
\* ggml-vulkan.dll

또한 build/src/Release(or Debug) 또는 build/ggml/src/Release(or Debug) 폴더 내에 생성된 다음 파일들을 **게임 서버 실행 파일(.exe)** 경로로 복사합니다

**복사 필수 lib 목록:**
* llama.lib
* ggml.lib
* ggml-base.lib
* ggml-cpu.lib

또한 include 폴더 내에 있는 파일들을 소스 코드 경로로 복사합니다.

**복사 필수 헤더 목록:**
llama.h
llama-cpp.h
---




## 🚀 5. 실행 테스트

모델 파일(.gguf)을 준비한 후 아래 명령어로 서버 구동을 확인합니다.

\# 1. CLI 기본 추론 테스트
./llama-cli -m models/your-model.gguf -p "Hello, AI Server!"

\# 2. 게임 서버 연동을 위한 API 서버 모드 실행
./llama-server -m models/your-model.gguf --port 8080

---
