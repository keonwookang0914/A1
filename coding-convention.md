# Project-A1

맵 생성 규칙
-

1. EmptyMap.umap은 defaultmap 설정을 위한 map으로 절대 건드리지 않는다
   - 내부 액터가 조금이라도 변경될 경우 git에 잡히는 데 .umap은 binary file이라 conflict 나기 때문
2. 각자 개발중인 맵은 Contents/Maps/DevMap안에 만들어 개발한다
   - 이는 추적되지 않기에 용이함
3. 개발 및 리뷰가 완료된 맵은 상위폴더인 Maps로 옮겨 git에 추가한다
   - 파일 명명방식 : 해당 맵의 특징을 살린 단어 + 수정 횟수를 의미하는 숫자
     - ex) 내부 우주선 맵의 초기 버전 -> InnnerMap0
   - git에 추가된 맵이 수정이 필요한 경우, 바로 수정하는 것이 아닌 해당 맵 파일을 복사해 사용한다.
     - ex) copy InnerMap0 -> InnerMap1
   - 수정된 맵 파일의 리뷰가 완료되면 버전에 해당하는 숫자를 1증가시키고 해당 맵보다 낮은 버전의 맵은 삭제한다
     - ex) delete InnerMap0, add InnerMap1


-----
블루프린트 생성 규칙
-

1. 각자 개발중인 맵은 Contents/Blueprints/Devoloping안에 만들어 개발한다
   - 이는 추적되지 않기에 용이함
3. 개발 및 리뷰가 완료된 맵은 상위폴더인 Blueprints로 옮겨 git에 추가한다
   - 파일 명명방식 : 해당 파일의 특징을 살린 단어 + 수정 횟수를 의미하는 숫자
     - ex) 캐릭터의 초기 버전 -> PlayerCharacter0
   - git에 추가된 파일이 수정이 필요한 경우, 바로 수정하는 것이 아닌 해당 파일을 복사해 사용한다.
     - ex) copy PlayerCharacter0 -> PlayerCharacter1
   - 수정된 맵 파일의 리뷰가 완료되면 버전에 해당하는 숫자를 1증가시키고 해당 맵보다 낮은 버전의 맵은 삭제한다
     - ex) delete PlayerCharacter0, add PlayerCharacter1


-----
Branch 네이밍 규칙
-
개발 목록 / 개발 기능
- ex) Monster/Battle