Smart Pillbox — 智慧藥盒提醒系統
學生姓名： 林郁荔
學號： B11400004
開發平台： STM32 NUCLEO-G474RE
微控制器： STM32G474RET6
開發環境： STM32CubeIDE / STM32CubeMX
程式語言： C（STM32 HAL Library）
專題類型： 微算機系統期中專題

1. 專題介紹（Project Overview）
本專題以 STM32 微控制器為核心，設計一套智慧藥盒提醒系統（Smart Pillbox Reminder System），透過 LED 燈號、按鈕確認與 UART 訊息輸出，模擬日常生活中的定時服藥提醒功能。

系統設定早餐與晚餐兩個服藥時段，利用 TIM2 計時器建立時間基準，並透過狀態機（State Machine）管理提醒、確認、逾時與漏服等狀態。

使用者可按下開發板上的 B1 按鈕確認服藥，系統會根據確認時間記錄服藥結果。

即使使用者沒有按下確認按鈕，系統也會在指定時間將該餐標記為 MISSED，並自動進入下一個服藥時段，避免提醒流程停滯。

本專題主要整合 GPIO、Timer、EXTI、NVIC、UART 與狀態機等微控制器應用技術。

2. 專題動機（Motivation）
日常生活中，長者、慢性病患者或需要定期服藥的人，可能因忙碌或健忘而錯過服藥時間。

一般提醒裝置雖然能發出通知，但不一定能記錄使用者是否完成服藥確認。

因此，本專題希望透過簡單的 LED 提醒與按鈕確認機制，建立一套容易操作的智慧藥盒原型。

系統除了提供服藥提醒，也能區分準時服藥、延遲服藥與未服藥三種結果，並透過 UART 顯示當日紀錄。

3. 使用設備（Hardware）
設備	用途
STM32 NUCLEO-G474RE	系統控制核心
麵包板	電路連接
綠色 LED	顯示已確認服藥
黃色 LED	顯示服藥提醒
紅色 LED	顯示服藥逾時
電阻	LED 限流
B1 USER Button	使用者確認服藥
杜邦線	連接電路
USB 傳輸線	程式下載、除錯與供電
3.1 腳位配置（Pin Configuration）
STM32 腳位	功能	說明
PC8	GPIO Output	綠色 LED
PC9	GPIO Output	黃色 LED
PC10	GPIO Output	紅色 LED
PC13	GPIO EXTI	開發板內建 B1 按鈕
TIM2	Timer Interrupt	每 1 ms 產生計時中斷
LPUART1	UART	輸出系統狀態與服藥紀錄
外接 LED 採用 GPIO 輸出 HIGH 時亮燈、LOW 時熄燈的方式控制。

B1 使用外部中斷偵測按鈕事件，並加入約 200 ms 的防彈跳處理。

4. 系統功能（System Features）
4.1 定時服藥提醒
系統設定早餐與晚餐兩個提醒時段。

為方便期中考試展示，使用秒數模擬一天的服藥流程。

時間	系統事件	LED 狀態
0 秒	系統啟動	全部熄滅
15 秒	早餐提醒	黃燈亮
25 秒	早餐逾時（未確認時）	紅燈亮
30 秒	早餐結束，未確認則記錄 MISSED	全部熄滅
40 秒	晚餐提醒	黃燈亮
50 秒	晚餐逾時（未確認時）	紅燈亮
55 秒	晚餐結束，未確認則記錄 MISSED	全部熄滅
60 秒	當日流程結束	全部熄滅
若使用者提前完成服藥確認，系統會記錄該餐結果，並等待下一個提醒時段。

4.2 LED 狀態顯示
系統透過三顆 LED 顯示不同的服藥狀態。

LED	狀態	意義
黃色	REMINDING	正在提醒服藥
綠色	CONFIRMED	已確認服藥
紅色	OVERDUE	已超過準時確認時間
全部熄滅	WAITING / 流程結束	等待下一個時段或完成當日流程
4.3 按鈕確認
使用者透過開發板內建 B1 按鈕確認服藥。

系統依照按鈕觸發時間，將服藥結果分為三種：

結果	說明
ON TIME	在提醒期間內完成確認
LATE	已逾時，但在該餐結束前完成確認
MISSED	該餐結束前未完成確認
同一個服藥時段只會記錄一次結果，避免重複按鈕造成重複確認。

4.4 自動切換與漏服紀錄
系統使用狀態機管理早餐與晚餐的提醒流程。

當使用者沒有按下 B1 時，系統不會永久停留在逾時狀態，而是依照設定時間自動執行：

REMINDING → OVERDUE → MISSED

早餐結束後，系統會繼續等待晚餐提醒時間。

晚餐結束後，系統會輸出當日服藥紀錄並結束流程。

4.5 UART 訊息輸出
系統透過 LPUART1 輸出提醒訊息、確認結果與當日服藥紀錄。

主要訊息包含：

SYSTEM STARTED

BREAKFAST REMINDER

DINNER REMINDER

OVERDUE

CONFIRMED

ON TIME

LATE

MISSED

DAY COMPLETED

UART 通訊設定為 115200 baud、8N1。

5. 系統流程（System Flow）
5.1 整體流程圖
flowchart TD
    A([系統啟動]) --> B[初始化 GPIO、TIM2、LPUART1]
    B --> C[開始系統計時]
    C --> D[等待早餐提醒時間]

    D --> E[15 秒：早餐提醒]
    E --> F{是否按下 B1?}

    F -->|準時確認| G[早餐 ON TIME]
    F -->|未確認| H[25 秒：早餐 OVERDUE]

    H --> I{30 秒前是否確認?}
    I -->|有| J[早餐 LATE]
    I -->|沒有| K[早餐 MISSED]

    G --> L[等待晚餐提醒時間]
    J --> L
    K --> L

    L --> M[40 秒：晚餐提醒]
    M --> N{是否按下 B1?}

    N -->|準時確認| O[晚餐 ON TIME]
    N -->|未確認| P[50 秒：晚餐 OVERDUE]

    P --> Q{55 秒前是否確認?}
    Q -->|有| R[晚餐 LATE]
    Q -->|沒有| S[晚餐 MISSED]

    O --> T[60 秒：輸出當日紀錄]
    R --> T
    S --> T

    T --> U([當日流程結束])
5.2 狀態機（State Machine）
系統使用 enum 定義五種主要狀態：

狀態	功能
WAITING	等待下一個服藥時段
REMINDING	黃燈提醒服藥
CONFIRMED	已按下按鈕確認
OVERDUE	超過準時確認時間
MISSED	該餐結束前未確認服藥
另外使用獨立變數儲存早餐與晚餐的服藥結果，避免不同時段的資料互相覆蓋。

6. 程式架構（Software Architecture）
6.1 主要程式模組
函式／變數	功能
system_ms	記錄系統經過的毫秒數
HAL_TIM_PeriodElapsedCallback()	TIM2 計時中斷回呼
HAL_GPIO_EXTI_Callback()	B1 按鈕外部中斷回呼
UART_Print()	傳送 UART 訊息
LED_Set()	控制三顆 LED
Start_Reminder()	開始服藥提醒
Set_Overdue()	切換至逾時狀態
Confirm_Medicine()	確認服藥並記錄結果
Mark_Missed()	記錄未服藥
Pillbox_Update()	管理系統時間與狀態轉換
Print_Daily_Report()	輸出當日服藥結果
6.2 Timer 計時
使用 TIM2 更新中斷建立 1 ms 計時基準。

每次 TIM2 中斷時，系統計時變數 system_ms 增加 1。

主程式透過比較 system_ms 與預設時間，判斷是否進入提醒、逾時或結束狀態。

6.3 外部中斷
使用 EXTI 偵測 B1 按鈕。

當按鈕觸發時，中斷回呼函式設定事件旗標，再由主迴圈處理服藥確認。

此方式可以避免在中斷函式內執行過多操作。

6.4 主迴圈
主迴圈主要負責：

檢查按鈕事件。

處理服藥確認。

檢查系統時間。

更新服藥狀態。

控制 LED。

輸出 UART 訊息。

7. 使用技術（Technical Implementation）
技術	使用情況	應用
GPIO Output	✓	控制三顆 LED
GPIO / EXTI	✓	B1 按鈕事件偵測
Timer Interrupt	✓	TIM2 毫秒計時
NVIC	✓	管理外部中斷與計時器中斷
RCC / PLL	✓	系統時脈設定
UART	✓	LPUART1 訊息輸出
State Machine	✓	服藥流程控制
ADC	—	未使用
PWM	—	未使用
DMA	—	未使用
I²C	—	未使用
SPI	—	未使用
RTC	—	未使用
8. UART 輸出範例（UART Output）
以下為預期輸出範例，假設早餐準時確認、晚餐完全未確認。

========================
SMART PILLBOX SYSTEM
SYSTEM STARTED
WAITING FOR BREAKFAST...
========================

[BREAKFAST] REMINDER
[CONFIRMED] MEDICINE TAKEN

[DINNER] REMINDER
[WARNING] OVERDUE
[DINNER] MISSED

===== DAILY REPORT =====
BREAKFAST: ON TIME
DINNER: MISSED
===== DAY COMPLETED =====
實際輸出會依照使用者的按鈕操作而改變。

9. 測試項目（Testing）
以下為新版系統的功能驗收項目。

測試項目	預期結果	實測結果
程式編譯	無編譯錯誤	待填寫
程式燒錄	成功下載至 STM32	待填寫
TIM2 計時	system_ms 每 1 ms 增加	待填寫
早餐提醒	15 秒黃燈亮	待填寫
早餐準時確認	綠燈亮，記錄 ON TIME	待填寫
早餐逾時	25 秒紅燈亮	待填寫
早餐延遲確認	記錄 LATE	待填寫
早餐未確認	30 秒記錄 MISSED	待填寫
晚餐提醒	40 秒黃燈亮	待填寫
晚餐逾時	50 秒紅燈亮	待填寫
晚餐未確認	55 秒記錄 MISSED	待填寫
自動切換	早餐未確認仍進入晚餐	待填寫
當日結束	60 秒輸出完整紀錄	待填寫
UART 訊息	正確顯示系統狀態	待填寫
9.1 實作照片
以下照片於實際測試後補上：

硬體接線

（放入 STM32、麵包板與 LED 接線照片）

早餐提醒

（放入黃色 LED 亮起照片）

確認服藥

（放入綠色 LED 亮起照片）

逾時提醒

（放入紅色 LED 亮起照片）

UART 輸出

（放入序列埠終端機或 Debugger 截圖）

10. 專案檔案（Project Files）
專案使用 STM32CubeIDE 建立。

StartPillbox/
│
├── Core/
│   ├── Inc/
│   │   └── main.h
│   │
│   └── Src/
│       ├── main.c
│       ├── gpio.c
│       ├── tim.c
│       └── usart.c
│
├── Drivers/
│
├── StartPillbox.ioc
│
└── README.md
主要檔案說明：

檔案	用途
main.c	智慧藥盒主要控制邏輯
gpio.c	GPIO 初始化
tim.c	TIM2 初始化
usart.c	LPUART1 初始化
main.h	主要標頭檔
StartPillbox.ioc	STM32CubeMX 設定檔
實際檔案結構以 STM32CubeIDE 產生的專案為準。

11. 後續改進方向（Future Improvements）
11.1 RTC 實際時間管理
目前使用秒數模擬一天的服藥流程。

未來可加入 RTC（Real-Time Clock），改為使用真實時間設定早餐與晚餐提醒。

11.2 可調整提醒時間
增加使用者設定功能，讓不同使用者可以依照自己的服藥習慣調整提醒時間。

11.3 服藥紀錄儲存
目前的服藥結果主要保存在程式執行期間的變數中。

未來可使用非揮發性記憶體儲存紀錄，避免系統重新啟動後資料遺失。

11.4 更多提醒方式
未來可加入蜂鳴器、LCD 顯示器或無線通訊模組，讓提醒方式更加完整。

12. 專題心得（Reflection）
透過本次智慧藥盒提醒系統專題，我將課堂學習的 GPIO、Timer、外部中斷與 UART 技術整合到實際應用中。

在程式設計過程中，我也進一步理解了狀態機與時間管理的重要性。相較於單純控制 LED 亮滅，智慧藥盒需要同時考慮提醒時間、按鈕事件、逾時判斷，以及不同服藥時段之間的切換。

其中，如何讓系統在使用者沒有按下按鈕時仍能繼續執行，是本次專題的重要設計重點。

透過這次實作，我對 STM32 微控制器的中斷處理、計時控制與程式流程設計有了更深入的認識，也累積了使用 STM32CubeIDE 進行嵌入式系統開發的經驗。
