tmux (Terminal MUltipleXer)
===

### Session 관련 tmux 명령어

- 세션 생성 (이름은 숫자로 생성됨)

`$ tmux`

-  이름을 지정하여 세션 생성

`$ tmux new -s <session_name>`

`$ tmux new-session -s <session_name>`

- 세션 이름 수정

`[Ctrl] + b,  $`

- 세션 detach

`[Ctrl] + b,  d`

- 세션 리스트보기

`$ tmux ls`

- 세션 attach

`$ tmux attach -t <session number 혹은 session name>`

- 세션 종료, 세션의 마지막 윈도우, 마지막 팬에서 실행

`$ exit `

- 세션 종료, 세션 밖에서 실행

`$ tmux kill-session -t session_name`
---
### Window 관련 tmux 명령어

- 세션 생성 (이름은 숫자로 생성됨)

`$ tmux`

- 이름을 지정하여 세션 생성

`$ tmux new -s <session_name>`

`$ tmux new-session -s <session_name>`

- 세션 이름 수정

`[Ctrl] + b,  $`

- 세션 detach

`[Ctrl] + b,  d`

- 세션 리스트보기

`$ tmux ls`

- 세션 attach

`$ tmux attach -t <session number 혹은 session name>`

- 세션 종료, 세션의 마지막 윈도우, 마지막 팬에서 실행

`$ exit`

- 세션 종료, 세션 밖에서 실행

`$ tmux kill-session -t session_name`
---
### Pane(팬) 관련 tmux 명령어

- 세로 화면 분할

`[Ctrl] + b, %`

- 가로 화면 분할

`[Ctrl] + b, "`

- 팬 이동 - 화면에 나오는 숫자로 이동

`[Ctrl] + b, q`

- 팬 이동 - 순서대로 이동

`[Ctrl] + b, o`

- 팬 이동 - 방향키로 이동

`[Ctrl] + b, <방향키>`

- 팬 삭제

`[Ctrl] + d`

`[Ctrl] + b, x`

- 팬 사이즈 조절 - 현재 포커스된 팬 전체화면(한번 더 실행하면 윈상복구)

`[Ctrl] + b, z`

- 팬 사이즈 조절 [Ctrl] + b 를 누른 후 :

`[Ctrl] + b, :`

`resize-pane -L <Size> or -R <Size> or -U <Size> -D <Size>`

- 팬 레이아웃 변경 (다양한 레이아웃으로 자동 전환)

`[Ctrl] + b, spacebar`
---
### 단축키 관련 명령

- 단축키 목록

`$ [Ctrl] + b, ?`

- 키 바인딩 및 언바인딩

`[Ctrl] + b, :`

`bind-key [-cnr] [-t key-table] key command [arguments]`

`unbind-key [-acn] [-t key-table] key`

- 옵션 설정 - set-option

`[Ctrl] + b, :`

`set -g <option-name> <option-value>`

- 옵션 설정 - set-window-option

`[Ctrl] + b, :`

`setw -g <option-name> <option-value>`

- 열려있는 모든 팬에 동시 입력하기

`[Ctrl] + b, :`

`setw synchronize-panes on`
---
### 스크롤링

- Copy 모드로 들어가기

`[Ctrl] + b, [`

- 빠져나오기

`[ESC]`

`q`
