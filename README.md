# **[UE4] 🔮 Otro Mundo**


## 📜 프로젝트 소개
이 프로젝트는 5명의 NPC와 협력하여 던전을 공략하는 RPG입니다.

## 🙋🏻‍♀️ 개발 인원
1명

## ⌛ 개발 기간
6개월

## 🎞️ 게임 영상
[Otro Mundo(YUTUBE)](https://youtu.be/MVgG-HYuBYA)


## 📍 대화 시스템

![talk1](https://github.com/user-attachments/assets/da89bbde-5eca-4ac2-8d9d-eadaf044c114)


대사가 한 글자씩 순차적으로 출력되는 것처럼 보이게 구현했으며


NPC의 머리 위에 나타나는 말풍선을 통해 누구의 대사인지 알 수 있다.


말하고 있는 NPC는 입모양도 움직인다.


![talk2](https://github.com/user-attachments/assets/339f3da5-ea93-4dd0-86d8-ffdf5f9bf81a)


플레이어는 1~3개의 선택지 중 하나를 선택해 응답할 수 있다.



## 📍 타겟팅(자동, 수동)

![ManualTargeting](https://github.com/user-attachments/assets/d2109248-4db4-493c-8d64-8fff865d93c6)


Tab키를 눌러 플레이어의 전투 범위 내에 들어와있는 적을 수동으로 타켓팅할 수 있다.


Tab키를 한 번 더 누르면 범위 내의 다른 적으로 타겟을 변경할 수도 있다.

타겟팅된 적은 머리 위에 노란색 화살표와 체력 상태가 표시된다.


![AutoTargeting](https://github.com/user-attachments/assets/cf0c4270-50a9-4a7c-a967-28a26619a661)


타겟을 지정하지 않은 상태에서 공격이 적중할 경우


해당 적이 자동으로 타겟팅된다.


## 📍 아이템 상호작용
![interact1](https://github.com/user-attachments/assets/153444c2-1d9c-42c6-8cf3-2522a94ee17c)
![interact2](https://github.com/user-attachments/assets/1614874e-a43c-4e5b-b814-b4c8e08ab145)
![interact3](https://github.com/user-attachments/assets/fbab0131-c1c9-4981-91d9-30cfac7715a3)
![interact4](https://github.com/user-attachments/assets/56fc4118-47f5-40b2-873e-11b608a9d758)


플레이어의 아이템 상호작용 범위에 특정 액터가 오버랩되어있을 경우


마우스 왼쪽 클릭으로 아이템을 집는 등의 상호작용을 할 수 있다.


## 📍 저장/로드

![save1](https://github.com/user-attachments/assets/40562bfd-9f7d-4c1e-85b7-5c30aefc6eb3)
![save2](https://github.com/user-attachments/assets/a5c45047-2615-483c-a74a-f0fda0641d84)


매 초 게임의 진행 상태를 자동으로 저장하도록 되어있어서

타이틀의 이어하기 메뉴를 통해 저장된 캐릭터들의 위치, 플레이어의 스탯, 게임의 진행상황을 불러올 수 있다.

