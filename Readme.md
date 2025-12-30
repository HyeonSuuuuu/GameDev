https://grandiose-delphinium-56f.notion.site/2649f08c004a81e49fa9f14f5815fdd5?source=copy_link


## 컨벤션

- 스크립트
    - c++ type 컨벤션(int32, int64)
    - 함수명 : 파스칼 표기법
    - 멤버 변수 : _변수명 (카멜) or m_ 카멜
    - 파일 및 폴더명 : 파스칼 표기법
    - 클래스명 : 파스칼 표기법
    - 함수 내 변수명 : 카멜 표기법
    - using namespace; X (std도 신중히)
- 커밋
    - Feat: 새로운 기능 추가
    - Fix: 버그 수정
    - Docs: 문서 수정
    - Style: 코드 포맷팅, 스타일 변경
    - Refactor: 코드 리팩터링
    - Test: 테스트 추가/수정
    - Chore: 빌드 업무, 기타 수정 (코드 기능이나 버그 수정과는 관련 없는 유지보수 작업)


```markdown
# C++ Core Guidelines Code Review Checklist

## Resource Management (R)
- [ ] 모든 new는 make_unique/make_shared로 교체 가능한가?
- [ ] 모든 raw pointers가 명확한 소유권을 가지는가?
- [ ] 모든 자원이 RAII로 관리되는가?

## Functions (F)
- [ ] 함수 매개변수의 소유권이 명확한가?
- [ ] 큰 객체를 const 참조로 전달하는가?
- [ ] 함수가 수정하지 않는 인자에 const를 사용하는가?

## Type System (T)
- [ ] auto로 타입을 충분히 활용하는가?
- [ ] 타입 변환이 명시적인가?
- [ ] 강타입(strong types)을 정의할 기회가 있는가?

## Error Handling (E)
- [ ] 오류 조건을 예외로 표현하는가?
- [ ] RAII와 예외가 조화롭게 작동하는가?
- [ ] 예외 안전성 보장(basic, strong, nothrow)이 있는가?

## Concurrency (CP)
- [ ] 모든 데이터 경쟁이 제거되었는가?
- [ ] std::thread가 RAII로 관리되는가?
- [ ] 데드락 위험이 없는가?
```
