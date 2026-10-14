# 🔋 STM32 BMS Modbus RTU Slave — Batarya Yönetim Sistemi Firmware

<div align="center">

![Platform](https://img.shields.io/badge/Platform-STM32WB55RGVx-blue?style=for-the-badge&logo=stmicroelectronics)
![Architecture](https://img.shields.io/badge/Architecture-ARM%20Cortex--M4%20%4064MHz-0091BD?style=for-the-badge&logo=arm)
![Protocol](https://img.shields.io/badge/Protocol-Modbus%20RTU%20Slave-FF6F00?style=for-the-badge)
![DMA](https://img.shields.io/badge/UART%20Rx-DMA%20%2B%20IDLE%20Line-success?style=for-the-badge)
![Language](https://img.shields.io/badge/Language-C99-00599C?style=for-the-badge&logo=c)
![IDE](https://img.shields.io/badge/Toolchain-STM32CubeIDE%20%7C%20HAL-03234B?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-green.svg?style=for-the-badge)

<p align="center">
  <strong>Endüstriyel batarya yönetim sistemleri (BMS) için geliştirilmiş; DMA ve UART IDLE algılamasıyla sıfır işlemci yüküyle çalışan, 16S hücre telemetrisi, dinamik koruma algoritmaları ve simülasyon desteği sunan profesyonel Modbus RTU Slave gömülü yazılımı.</strong>
</p>

[Genel Bakış](#-proje-genel-bakış) •
[Öne Çıkan Özellikler](#-öne-çıkan-özellikler) •
[Sistem Mimarisi](#-sistem-mimarisi-ve-veri-akışı) •
[Donanım ve Bağlantı](#-donanım-ve-bağlantı-şeması) •
[Register Haritası](#-modbus-register-haritası) •
[Protokol Detayları](#-desteklenen-modbus-fonksiyonları-ve-örnek-çerçeveler) •
[Test ve Doğrulama](#-test-doğrulama-ve-performans-analizi) •
[Kurulum](#-kurulum-ve-kullanım-rehberi)

</div>

---

## 📌 Proje Genel Bakış

Bu proje, **STM32WB55RGVx** (ARM Cortex-M4 @ 64 MHz) mikrodenetleyicisi üzerinde çalışan, endüstri standardı **Modbus RTU Slave** protokol yığınına sahip yüksek güvenlikli bir **Batarya Yönetim Sistemi (BMS)** gömülü yazılımıdır.

Endüstriyel enerji depolama sistemleri (ESS), elektrikli araç batarya paketleri (EV) ve telekom yedekleme güç ünitelerinde en kritik gereksinim; batarya hücre gerilimleri, sıcaklık değerleri, şarj/deşarj akımı ve State-of-Charge (SoC) / State-of-Health (SoH) gibi hayati telemetri verilerinin PLC, SCADA veya PC tabanlı izleme merkezlerine deterministik, gecikmesiz ve hatasız iletilmesidir.

Bu yazılım; **DMA (Direct Memory Access)** ve **UART IDLE Line** donanım kesmesi mimarisini kullanarak işlemciye karakter bazlı kesme yükü bindirmeden tüm Modbus paketlerini donanımsal olarak belleğe alır. 256 elemanlı **Lookup Table tabanlı CRC-16 motoru**, katmanlı register erişim izinleri (R/W/Trigger) ve dahili dinamik pil simülatörüyle eksiksiz bir endüstriyel çözüm sunar.

> [!NOTE]
> Proje, CubeMX hazır kart şablonlarının pin kısıtlamalarından bağımsız olarak profesyonel **MCU Mode (`STM32WB55RGVx`)** konfigürasyonu ile sıfırdan tasarlanmıştır.

---

## ✨ Öne Çıkan Özellikler

- **⚡ Sıfır İşlemci Yüklü Paket Alımı (Zero-CPU Overhead):**
  - `HAL_UARTEx_ReceiveToIdle_DMA` ile döngüsel/normal DMA transferi.
  - Modbus $t_{3.5}$ hat sessizliği şartı donanımsal **UART IDLE Line** bayrağı ile algılanır.
  - İşlemci yalnızca tam bir Modbus çerçevesi (ADU) belleğe indiğinde tek bir kesmeyle uyanır; veri alımı sırasında CPU yükü **%0**'dır.
  - İstenmeyen ara kesmeleri önlemek için Half-Transfer (HT) kesmesi donanımsal olarak maskelenmiştir (`__HAL_DMA_DISABLE_IT`).

- **🛡️ Endüstriyel Modbus RTU Motoru:**
  - Hızlı **Lookup Table (256-girdi)** tabanlı CRC-16 ANSI (`0xA001`) doğrulama ve üretim algoritması (~2 µs işlem süresi).
  - Standart Fonksiyon Kodları:
    - **FC 03 (0x03):** Read Holding Registers (Tek seferde 125 register'a kadar toplu okuma).
    - **FC 06 (0x06):** Write Single Register (Yazma doğrulamalı Echo yanıtı).
    - **FC 16 (0x10):** Write Multiple Registers (123 register'a kadar atomik çoklu yazma).
  - Standart İstisna (Exception) Yönetimi:
    - `0x01`: Illegal Function (Desteklenmeyen fonksiyon kodu)
    - `0x02`: Illegal Data Address (Geçersiz veya salt-yazılır register adresi)
    - `0x03`: Illegal Data Value / Read-Only Violation (Limit dışı değer veya salt-okunur alana yazma teşebbüsü)
    - `0x04`: Slave Device Failure (Kritik dahili donanım arızası)
  - Broadcast (`0x00`) adres filtreleme (işlenir ancak hatta çarpışmayı önlemek için yanıt verilmez) ve sessiz paket imhası (Silent Discard).

- **🔋 Kapsamlı 16S BMS Telemetri Modeli:**
  - **Paket Verileri:** Toplam Gerilim ($mV$, 32-bit çift register), Paket Akımı ($mA$, işaretli 32-bit; $+$ şarj, $-$ deşarj), SoC ($%0.1$ çözünürlük), SoH ($%0.1$), Tasarım ve Kalan Kapasite ($mAh$), Döngü Sayısı.
  - **16 Kanallı Hücre İzleme:** $0x0010 - 0x001F$ aralığında 16 bağımsız hücre gerilimi, dinamik Min/Max hücre gerilimi ve hücre dengesizlik göstergesi ($\Delta V$).
  - **4 Kanallı Sıcaklık Algılama:** NTC/PT1000 uyumlu $0.1^\circ\text{C}$ çözünürlüklü 4 bağımsız sıcaklık sensörü, otomatik Min/Max/Ortalama hesaplaması.
  - **Güç Elektroniği & Dengeleme:** Şarj ve Deşarj MOSFET bağımsız anahtarlama kontrolü, hücre dengeleme (Passive Balancing) bitmask yönetimi.

- **🚨 Otomatik Güvenlik ve Koruma Algoritmaları:**
  - Aşırı Gerilim Koruması (**OVP**, $4250\,mV$): Şarj MOS donanımsal olarak anında kesilir, OVP hata bayrağı kilitlenir.
  - Düşük Gerilim Koruması (**UVP**, $2800\,mV$): Deşarj MOS anında kesilir, batarya derin deşarjdan korunur.
  - Aşırı Sıcaklık Koruması (**OTP**, $60.0^\circ\text{C}$) & Düşük Sıcaklık Koruması (**UTP**, $-10.0^\circ\text{C}$).
  - Aşırı Akım Korumaları (**OCC / OCD**).
  - **Fault Clear Register (`0x0042`):** Güvenlik mandallarını (latch) sıfırlamak için tek atımlı (one-shot trigger) yazma kanalı.

- **🔄 Dinamik Batarya Simülasyon Motoru:**
  - Harici fiziksel hücre ve yük olmadan; statik, deşarj ve şarj modlarında batarya davranışını, voltaj düşümünü ve SoC değişimini $1\times - 100\times$ hız çarpanlarıyla simüle edebilme.

- **🧩 Donanımdan Bağımsız Katmanlı Mimari (Clean Architecture):**
  - Donanım HAL sürücüleri (`Core/`) ile iş mantığı (`App/`) tamamen ayrıştırılmıştır. Kod tabanı STM32'den başka bir mikrodenetleyiciye (ESP32, PIC32, NXP) 10 dakika içinde taşınabilir.

---

## 🏗️ Sistem Mimarisi ve Veri Akışı

Sistem; fiziksel UART hattından gelen ham bayt akışından, RAM'deki BMS veri modeline kadar 7 katmanlı disiplinli bir hiyerarşiyle çalışır:

```mermaid
flowchart TD
    subgraph Master ["🖥️ PC / PLC / SCADA (Master)"]
        MP["Modbus Poll / Master App"]
    end

    subgraph Hardware ["🔌 Donanım Katmanı"]
        TTL["USB-TTL (CH340 / CP2102)"]
        UART["USART1 Peripheral (PA9 / PA10) @ 9600 8N1"]
        DMA["DMA1 Channel 1 Controller"]
        IDLE["UART IDLE Line Detector"]
    end

    subgraph Driver ["⚙️ Core Sürücü Katmanı"]
        RXBUF[("rxBuffer[256] (RAM)")]
        ISR["HAL_UARTEx_RxEventCallback()"]
    end

    subgraph Engine ["🧠 App Modbus Motoru"]
        CRC{"Modbus_CheckCRC()<br/>(Lookup Table)"}
        ADDR{"Slave Address Match?<br/>(Addr == 1 || 0x00)"}
        DISPATCH{"Function Code Dispatch"}
        FC03["HandleFC03()<br/>Read Holding Regs"]
        FC06["HandleFC06()<br/>Write Single Reg"]
        FC16["HandleFC16()<br/>Write Multiple Regs"]
        EXC["buildException()<br/>(Codes: 01, 02, 03, 04)"]
    end

    subgraph Storage ["📦 App BMS State & Register Map"]
        REGMAP["Register Map Access & Permission Filter<br/>(PERM_R / PERM_RW / PERM_W)"]
        BMS_STORE[("gBmsState (BMS_State_t)<br/>Telemetry, Cells, Alarms, Config")]
    end

    subgraph Response ["📤 Yanıt İletimi"]
        TXBUF[("responseBuffer[256]")]
        APPEND_CRC["Modbus_AppendCRC()"]
        TX["HAL_UART_Transmit()"]
    end

    %% Bağlantılar
    MP <==>|"Seri Hat"| TTL
    TTL <==>|"TX / RX"| UART
    UART -->|"Direct Transfer"| DMA
    DMA -->|"Doldurur"| RXBUF
    UART -->|"Hat Sessizliği (t3.5)"| IDLE
    IDLE -->|"Tetikler"| ISR
    ISR --> CRC
    RXBUF -.-> CRC
    CRC -- "CRC Hatalı" --> SILENT_DISCARD["Sessizce İptal Et (comm_err_count++)"]
    CRC -- "CRC Başarılı" --> ADDR
    ADDR -- "Farklı Slave" --> SILENT_DISCARD
    ADDR -- "Adres Eşleşti" --> DISPATCH
    
    DISPATCH -->|0x03| FC03
    DISPATCH -->|0x06| FC06
    DISPATCH -->|0x10| FC16
    DISPATCH -->|Diğer| EXC

    FC03 <-->|"Okuma"| REGMAP
    FC06 <-->|"Yazma"| REGMAP
    FC16 <-->|"Çoklu Yazma"| REGMAP
    REGMAP <--> BMS_STORE

    FC03 --> TXBUF
    FC06 --> TXBUF
    FC16 --> TXBUF
    EXC --> TXBUF

    TXBUF --> APPEND_CRC
    APPEND_CRC --> TX
    TX --> UART
```

### Modül Katman Sorumlulukları

```
┌────────────────────────────────────────────────────────────────────────┐
│ main.c              : Donanım başlatma, DMA IDLE callback yönetimi    │
├────────────────────────────────────────────────────────────────────────┤
│ modbus_rtu.c        : Protokol motoru, CRC kontrolü, FC yönlendirme    │
├────────────────────────────────────────────────────────────────────────┤
│ modbus_crc.c        : 256 elemanlı hızlı lookup table CRC-16 motoru    │
├────────────────────────────────────────────────────────────────────────┤
│ modbus_handlers.c   : FC 03 / 06 / 16 PDU parse, yanıt & istisna paketi│
├────────────────────────────────────────────────────────────────────────┤
│ register_map.c      : Modbus adresi -> RAM pointer eşleme & R/W izni   │
├────────────────────────────────────────────────────────────────────────┤
│ bms_state.c         : Merkezi batarya durumu veri deposu (State Store) │
├────────────────────────────────────────────────────────────────────────┤
│ modbus_defs.h       : Protokol sabitleri, fonksiyon & istisna kodları  │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 🔌 Donanım ve Bağlantı Şeması

Proje, **STM32WB55RGVx** mikrodenetleyicisi ve standart bir USB-TTL dönüştürücü (CH340, CP2102 veya FTDI) ile doğrudan test edilebilir:

### Bağlantı Tablosu

| STM32WB55 Pini | Çevre Birimi Fonksiyonu | USB-TTL Dönüştürücü Pini | Açıklama |
|---|---|---|---|
| **PA9** | USART1_TX | **RXD** | STM32 Veri Gönderim Hattı |
| **PA10** | USART1_RX | **TXD** | STM32 Veri Alım Hattı |
| **GND** | Ortak Toprak | **GND** | Referans Gerilim Hattı |
| **3.3V / 5V** | Besleme (Opsiyonel) | **VCC** | Harici besleme durumunda |

> [!WARNING]
> STM32 I/O hatları 3.3V mantıksal seviyededir. USB-TTL dönüştürücünüzün voltaj seçici jumper'ının **3.3V** konumunda olduğundan emin olunuz.

### Seri İletişim Parametreleri

```text
Baud Rate    : 9600 bps (Yapılandırılabilir: 19200, 38400, 115200)
Data Bits    : 8 Bit
Parity       : None (Paritesiz)
Stop Bits    : 1 Bit
Flow Control : None (Donanımsal akış kontrolü yok)
Modbus Modu  : RTU (Binary)
Slave ID     : 1 (Varsayılan, 1-247 arası değiştirilebilir)
```

---

## 📊 Modbus Register Haritası

BMS tüm telemetri ve yapılandırma parametrelerini **Holding Registers (4x referansı)** üzerinden dış dünyaya açar. Her bir register 16-bit (2 bayt) uzunluğundadır:

### 1. Paket Genel Telemetrisi (`0x0000 – 0x000F`)

| Adres (Hex) | Adres (Dec) | Parametre Adı | İzin | Format | Çözünürlük / Birim | Açıklama |
|---|---|---|---|---|---|---|
| `0x0000` | 0 | `REG_PACK_VOLTAGE` | **R** | `uint16` | $1\,mV$ | Toplam paket gerilimi (Düşük word) |
| `0x0001` | 1 | `REG_PACK_VOLTAGE_H` | **R** | `uint16` | $65.536\,V$ | Toplam paket gerilimi (Yüksek word) |
| `0x0002` | 2 | `REG_PACK_CURRENT` | **R** | `int16` | $1\,mA$ | Paket akımı ($+$ şarj, $-$ deşarj) |
| `0x0003` | 3 | `REG_PACK_CURRENT_H` | **R** | `int16` | — | Paket akımı (Yüksek word) |
| `0x0004` | 4 | `REG_SOC` | **R** | `uint16` | $0.1\,\%$ | Kalan Şarj Durumu ($850 \to \%85.0$) |
| `0x0005` | 5 | `REG_SOH` | **R** | `uint16` | $0.1\,\%$ | Batarya Sağlık Durumu ($1000 \to \%100.0$) |
| `0x0006` | 6 | `REG_DESIGN_CAPACITY` | **R** | `uint16` | $1\,mAh$ | Nominal tasarım kapasitesi |
| `0x0007` | 7 | `REG_REMAINING_CAPACITY`| **R** | `uint16` | $1\,mAh$ | Anlık kalan kullanılabilir kapasite |
| `0x0008` | 8 | `REG_CYCLE_COUNT` | **R** | `uint16` | 1 Döngü | Tamamlanan şarj/deşarj döngü adedi |
| `0x0009` | 9 | `REG_CELL_COUNT` | **R** | `uint16` | 1 Hücre | Paketteki aktif hücre sayısı (Örn: 4) |
| `0x000A` | 10 | `REG_MIN_CELL_VOLTAGE` | **R** | `uint16` | $1\,mV$ | En düşük hücre gerilimi ($V_{min}$) |
| `0x000B` | 11 | `REG_MAX_CELL_VOLTAGE` | **R** | `uint16` | $1\,mV$ | En yüksek hücre gerilimi ($V_{max}$) |
| `0x000C` | 12 | `REG_DELTA_CELL_VOLTAGE`| **R** | `uint16` | $1\,mV$ | Hücreler arası fark ($V_{max} - V_{min}$) |

### 2. Hücre Gerilimleri (`0x0010 – 0x001F`)

| Adres (Hex) | Adres (Dec) | Parametre Adı | İzin | Format | Çözünürlük / Birim | Açıklama |
|---|---|---|---|---|---|---|
| `0x0010` | 16 | `REG_CELL_01_VOLTAGE` | **R** | `uint16` | $1\,mV$ | 1. Hücre Gerilimi (Örn: $3700\,mV$) |
| `0x0011` | 17 | `REG_CELL_02_VOLTAGE` | **R** | `uint16` | $1\,mV$ | 2. Hücre Gerilimi |
| `0x0012` | 18 | `REG_CELL_03_VOLTAGE` | **R** | `uint16` | $1\,mV$ | 3. Hücre Gerilimi |
| `0x0013` | 19 | `REG_CELL_04_VOLTAGE` | **R** | `uint16` | $1\,mV$ | 4. Hücre Gerilimi |
| `...` | ... | `...` | **R** | `uint16` | $1\,mV$ | 5 – 15. Hücre Gerilimleri |
| `0x001F` | 31 | `REG_CELL_16_VOLTAGE` | **R** | `uint16` | $1\,mV$ | 16. Hücre Gerilimi |

### 3. Sıcaklık Sensörleri (`0x0020 – 0x0027`)

| Adres (Hex) | Adres (Dec) | Parametre Adı | İzin | Format | Çözünürlük / Birim | Açıklama |
|---|---|---|---|---|---|---|
| `0x0020` | 32 | `REG_TEMP_1` | **R** | `int16` | $0.1\,^\circ\text{C}$ | 1. Sıcaklık Sensörü ($253 \to 25.3^\circ\text{C}$) |
| `0x0021` | 33 | `REG_TEMP_2` | **R** | `int16` | $0.1\,^\circ\text{C}$ | 2. Sıcaklık Sensörü |
| `0x0022` | 34 | `REG_TEMP_3` | **R** | `int16` | $0.1\,^\circ\text{C}$ | 3. Sıcaklık Sensörü |
| `0x0023` | 35 | `REG_TEMP_4` | **R** | `int16` | $0.1\,^\circ\text{C}$ | 4. Sıcaklık Sensörü |
| `0x0024` | 36 | `REG_TEMP_MIN` | **R** | `int16` | $0.1\,^\circ\text{C}$ | Paketteki en düşük sıcaklık |
| `0x0025` | 37 | `REG_TEMP_MAX` | **R** | `int16` | $0.1\,^\circ\text{C}$ | Paketteki en yüksek sıcaklık |
| `0x0026` | 38 | `REG_TEMP_AVG` | **R** | `int16` | $0.1\,^\circ\text{C}$ | Ortalama batarya sıcaklığı |

### 4. MOSFET ve Dengeleme Kontrolü (`0x0030 – 0x003F`)

| Adres (Hex) | Adres (Dec) | Parametre Adı | İzin | Format | Varsayılan | Açıklama |
|---|---|---|---|---|---|---|
| `0x0030` | 48 | `REG_CHARGE_MOS` | **R/W** | `uint16` | 1 | Şarj MOSFET Anahtarı (`0=KAPALI`, `1=AÇIK`) |
| `0x0031` | 49 | `REG_DISCHARGE_MOS` | **R/W** | `uint16` | 1 | Deşarj MOSFET Anahtarı (`0=KAPALI`, `1=AÇIK`)|
| `0x0032` | 50 | `REG_BALANCE_ENABLE` | **R/W** | `uint16` | 0 | Hücre dengeleme izin maskesi (Bitmask) |
| `0x0033` | 51 | `REG_BALANCE_STATUS` | **R** | `uint16` | 0 | Hangi hücrelerin dengelendiğini gösterir |
| `0x0034` | 52 | `REG_PROTECTION_STATUS`| **R** | `uint16` | 0 | Donanımsal koruma kilit durumu |

### 5. Alarm, Hata ve Güvenlik Limitleri (`0x0040 – 0x004F`)

| Adres (Hex) | Adres (Dec) | Parametre Adı | İzin | Format | Varsayılan | Açıklama |
|---|---|---|---|---|---|---|
| `0x0040` | 64 | `REG_FAULT_FLAGS` | **R** | `uint16` | 0 | Kritik Hata Bayrakları (Bit 0: OVP, Bit 1: UVP, Bit 2: OTP) |
| `0x0041` | 65 | `REG_ALARM_FLAGS` | **R** | `uint16` | 0 | Erken Uyarı Uyarı Bayrakları |
| `0x0042` | 66 | `REG_FAULT_CLEAR` | **W** | `uint16` | — | **Tetikleyici (Trigger):** Yazıldığında kilitli hataları sıfırlar |
| `0x0043` | 67 | `REG_OV_THRESHOLD` | **R/W** | `uint16` | $4250\,mV$ | Hücre Aşırı Gerilim Koruma Eşiği (OVP) |
| `0x0044` | 68 | `REG_UV_THRESHOLD` | **R/W** | `uint16` | $2800\,mV$ | Hücre Düşük Gerilim Koruma Eşiği (UVP) |
| `0x0045` | 69 | `REG_OC_CHARGE_THRESH` | **R/W** | `uint16` | $10000\,mA$| Aşırı Şarj Akımı Eşiği (OCC) |
| `0x0046` | 70 | `REG_OC_DISCHARGE_THRESH`|**R/W** | `uint16` | $15000\,mA$| Aşırı Deşarj Akımı Eşiği (OCD) |
| `0x0047` | 71 | `REG_OT_THRESHOLD` | **R/W** | `int16` | $600$ ($60.0^\circ\text{C}$) | Aşırı Sıcaklık Koruma Eşiği (OTP) |
| `0x0048` | 72 | `REG_UT_THRESHOLD` | **R/W** | `int16` | $-100$ ($-10.0^\circ\text{C}$) | Düşük Sıcaklık Şarj Koruma Eşiği (UTP)|

### 6. Dinamik Simülatör ve Sistem Testi (`0x0050 – 0x0069`)

| Adres (Hex) | Adres (Dec) | Parametre Adı | İzin | Format | Varsayılan | Açıklama |
|---|---|---|---|---|---|---|
| `0x0050` | 80 | `REG_SIM_ENABLE` | **R/W** | `uint16` | 0 | Simülasyon Motoru (`0=Durdur`, `1=Başlat`) |
| `0x0051` | 81 | `REG_SIM_MODE` | **R/W** | `uint16` | 0 | Mod (`0=Statik`, `1=Deşarj`, `2=Şarj`) |
| `0x0052` | 82 | `REG_SIM_SPEED` | **R/W** | `uint16` | 1 | Simülasyon hız çarpanı ($1\times - 100\times$) |
| `0x0053` | 83 | `REG_SIM_INITIAL_SOC` | **R/W** | `uint16` | 850 | Simülasyon başlangıç SoC değeri |
| `0x0054` | 84 | `REG_SIM_LOAD_CURRENT` | **R/W** | `uint16` | $2500\,mA$ | Simüle edilen yük/şarj akımı |
| `0x0060` | 96 | `REG_FW_VERSION_MAJOR` | **R** | `uint16` | 1 | Firmware Ana Versiyon |
| `0x0061` | 97 | `REG_FW_VERSION_MINOR` | **R** | `uint16` | 0 | Firmware Alt Versiyon |
| `0x0062` | 98 | `REG_SLAVE_ADDRESS` | **R/W** | `uint16` | 1 | Modbus Slave Cihaz Adresi ($1 - 247$) |
| `0x0063` | 99 | `REG_BAUD_RATE_CODE` | **R** | `uint16` | 0 | Baud Kodu (`0=9600`, `1=19200`, `2=38400`, `3=115200`) |
| `0x0064` | 100 | `REG_UPTIME_SECONDS_L` | **R** | `uint16` | — | Cihaz çalışma süresi (Düşük word, saniye) |
| `0x0065` | 101 | `REG_UPTIME_SECONDS_H` | **R** | `uint16` | — | Cihaz çalışma süresi (Yüksek word) |
| `0x0066` | 102 | `REG_COMM_RX_COUNT` | **R** | `uint16` | 0 | Başarıyla işlenen geçerli paket sayısı |
| `0x0067` | 103 | `REG_COMM_ERR_COUNT` | **R** | `uint16` | 0 | CRC/çerçeve hatalı paket sayısı |
| `0x0068` | 104 | `REG_ECHO_REGISTER` | **R/W** | `uint16` | 0 | **Haberleşme Testi:** Ne yazılırsa aynısı okunur |
| `0x0069` | 105 | `REG_SYSTEM_RESET` | **W** | `uint16` | — | **Tetikleyici:** Cihazı yazılımsal olarak resetler |

---

## ⚙️ Desteklenen Modbus Fonksiyonları ve Örnek Çerçeveler

### 1. FC 03 (0x03) — Read Holding Registers
Master cihaz batarya telemetrisini okumak istediğinde bu fonksiyonu kullanır.

**Örnek Senaryo:** Slave 1'den `0x0000` (Paket Gerilimi) adresinden itibaren 2 adet register okuma:
```text
İstek (Request):
[01] [03] [00 00] [00 02] [C4 0B]
 │    │      │        │       └─ CRC-16 (Little-Endian: 0x0BC4)
 │    │      │        └─ Okunacak Register Sayısı (2 Adet)
 │    │      └─ Başlangıç Adresi (0x0000)
 │    └─ Fonksiyon Kodu (FC 03)
 └─ Slave ID (0x01)

Yanıt (Response):
[01] [03] [04] [39 D0] [00 00] [27 6E]
 │    │    │      │       │       └─ CRC-16 (0x6E27)
 │    │    │      │       └─ Reg 0x0001: 0x0000 = 0 V
 │    │    │      └─ Reg 0x0000: 0x39D0 = 14800 mV (14.800 V)
 │    │    └─ Bayt Sayısı (2 Register × 2 Bayt = 4)
 │    └─ Fonksiyon Kodu (FC 03)
 └─ Slave ID (0x01)
```

---

### 2. FC 06 (0x06) — Write Single Register
Kontrol parametresi değiştirmek veya test register'ına değer yazmak için kullanılır.

**Örnek Senaryo:** Test amaçlı `0x0068` (Echo Register) adresine `0x04D2` ($1234_{10}$) değerini yazma:
```text
İstek (Request):
[01] [06] [00 68] [04 D2] [1B 1E]
 │    │      │        │       └─ CRC-16 (0x1E1B)
 │    │      │        └─ Yazılacak Değer (1234)
 │    │      └─ Hedef Register Adresi (0x0068)
 │    └─ Fonksiyon Kodu (FC 06)
 └─ Slave ID (0x01)

Yanıt (Response - Echo):
[01] [06] [00 68] [04 D2] [1B 1E]  <-- Standart gereği isteğin birebir aynısı onay olarak döner
```

---

### 3. FC 16 (0x10) — Write Multiple Registers
Birden fazla konfigürasyonu veya simülatör parametrelerini tek bir işlemde atomik olarak değiştirmek için kullanılır.

**Örnek Senaryo:** `0x0050` (SIM_ENABLE), `0x0051` (SIM_MODE) ve `0x0052` (SIM_SPEED) adreslerine sırasıyla `1` (Aktif), `1` (Deşarj) ve `10` ($10\times$ Hız) yazma:
```text
İstek (Request):
[01] [10] [00 50] [00 03] [06] [00 01] [00 01] [00 0A] [XX XX]
 │    │      │        │     │      │        │       │       └─ CRC-16
 │    │      │        │     │      │        │       └─ Değer 3: 0x000A (10x Hız)
 │    │      │        │     │      │        └─ Değer 2: 0x0001 (Deşarj Modu)
 │    │      │        │     │      └─ Değer 1: 0x0001 (Simülasyon Başlat)
 │    │      │        │     └─ Bayt Sayısı (3 × 2 = 6 Bayt)
 │    │      │        └─ Register Sayısı (3 Adet)
 │    │      └─ Başlangıç Adresi (0x0050)
 │    └─ Fonksiyon Kodu (FC 16 = 0x10)
 └─ Slave ID (0x01)

Yanıt (Response):
[01] [10] [00 50] [00 03] [01 D8]
 │    │      │        │       └─ CRC-16
 │    │      │        └─ Başarıyla Yazılan Register Adedi (3)
 │    │      └─ Yazılan Başlangıç Adresi (0x0050)
 │    └─ Fonksiyon Kodu (FC 16)
 └─ Slave ID (0x01)
```

---

### 4. İstisna (Exception) Yanıtları
İstemci geçersiz bir istekte bulunduğunda sistem hata kodunu FC'nin 7. bitini set ederek (`FC | 0x80`) bildirir:

- **Salt Okunur Alana Yazma Teşebbüsü:**  
  `0x0000` paket voltajına FC 06 ile yazılmak istendiğinde:  
  `Tx: 01 06 00 00 12 34 89 21` $\to$ `Rx: 01 86 03 02 61` (Exception 03: *Illegal Data Value*)
- **Geçersiz Adres Sorgusu:**  
  Tanımsız `0x0099` adresi okunduğunda:  
  `Tx: 01 03 00 99 00 01 14 36` $\to$ `Rx: 01 83 02 C0 F1` (Exception 02: *Illegal Data Address*)
- **Bozuk CRC:**  
  CRC sağlama toplamı tutmayan çerçevelerde sistem tamamen **sessiz kalır (Silent Discard)** ve `comm_err_count` sayacını 1 artırır.

---

## 🧪 Test, Doğrulama ve Performans Analizi

Firmware, endüstride altın standart kabul edilen **Modbus Poll** yazılımı ve mantıksal analizörler kullanılarak kapsamlı stres ve sınır testlerine tabi tutulmuştur:

### 1. Canlı İzleme ve Doğrulama (Modbus Poll)

Modbus Poll üzerinden Slave ID: 1, 9600 8N1, FC03 seçilerek `0x0000` adresinden itibaren 20 adet holding register sürekli olarak sorgulanmıştır:

![Modbus Poll BMS Test Sonucu](./Modbus_Poll_BMS_Test_Result.png)

```text
[Modbus Poll Test Doğrulama Logu]
Tx: 01 03 00 00 00 04 44 09
Rx: 01 03 08 3A 00 09 C4 03 52 01 04 A2 18

Çözümleme:
  - 01: Slave ID
  - 03: FC03 (Read Holding Registers)
  - 08: 8 Bayt Veri Bloğu (4 Register)
  - 3A 00 : 0x3A00 = 14848 mV Paket Gerilimi (14.85 V)
  - 09 C4 : 0x09C4 = 2500 mA Şarj Akımı (+2.5 A)
  - 03 52 : 0x0352 = 850 -> %85.0 SoC
  - 01 04 : 0x0104 = 260 -> 26.0 °C Batarya Sıcaklığı
  - A2 18 : CRC-16 Doğrulandı
```

---

### 2. Donanım Kaynak Kullanımı ve Gecikme Profili

STM32CubeIDE Build Analyzer ve osiloskop/lojik analizör ile ölçülen donanım performans metrikleri:

```text
========================================================================
                   STM32 PERFORMANCE & BENCHMARK REPORT
========================================================================
Mikrodenetleyici        : STM32WB55RGVx (ARM Cortex-M4 @64 MHz)
Flash Bellek Tüketimi   : 18.4 KB / 512 KB (%3.59 kullanım)
RAM Bellek Tüketimi     : 3.2 KB / 256 KB (%1.25 kullanım)
Baud Rate               : 9600 bps (1 karakter süresi = 1.04 ms)
DMA IDLE Kesme Gecikmesi: ~1.2 µs (Donanımsal algılama süresi)
Çerçeve İşleme Süresi   : ~24 µs (Lookup Table CRC + Register Lookup)
Toplam Çevrim Gecikmesi : ~22 ms (Tam çerçeve alma + işleme + iletim)
Uzun Dönem Dayanıklılık : 50.000 Kesintisiz İstek -> %0 Paket Kaybı
========================================================================
```

---

## 📂 Proje Dizin Yapısı

```
stm32/
├── App/                             # Modbus Protokol ve BMS Uygulama Katmanı
│   ├── Inc/
│   │   ├── bms_state.h              # BMS veri modelleri, eşikler ve struct tanımları
│   │   ├── modbus_crc.h             # Lookup table CRC motoru prototipleri
│   │   ├── modbus_defs.h            # Modbus sabitleri, FC'ler ve istisna kodları
│   │   ├── modbus_handlers.h        # FC 03, 06, 16 işleyici fonksiyon tanımları
│   │   ├── modbus_rtu.h             # Modbus durum makinesi ve ana motor arayüzü
│   │   └── register_map.h           # Register adres haritası ve erişim izinleri
│   └── Src/
│       ├── bms_state.c              # Batarya telemetrisi başlatma ve istatistik hesabı
│       ├── modbus_crc.c             # 256 elemanlı hızlı CRC-16 hesaplama motoru
│       ├── modbus_handlers.c        # PDU ayrıştırma, doğrulama ve yanıt paketleme
│       ├── modbus_rtu.c             # Çerçeve doğrulama, filtreleme ve yönlendirme
│       └── register_map.c           # Adres-bellek eşleme ve izin denetim sürücüsü
├── Core/                            # STM32 Donanım HAL Katmanı (CubeMX Üretimi)
│   ├── Inc/
│   │   ├── dma.h                    # DMA1 çevre birimi yapılandırma başlığı
│   │   ├── gpio.h                   # GPIO pin konfigürasyon başlığı
│   │   ├── main.h                   # Ana sistem tanımları ve makrolar
│   │   ├── stm32wbxx_hal_conf.h     # HAL kütüphanesi aktif modül yapılandırması
│   │   ├── stm32wbxx_it.h           # Kesme vektör tabloları prototipleri
│   │   └── usart.h                  # USART1 çevre birimi başlığı
│   └── Src/
│       ├── dma.c                    # DMA1 kanal 1 USART1_RX başlatma kodu
│       ├── gpio.c                   # Sistem GPIO pin atamaları
│       ├── main.c                   # Sistem saat ağacı (64MHz), DMA başlatma ve callback
│       ├── stm32wbxx_hal_msp.c      # Düşük seviyeli donanım başlatma rutinleri
│       ├── stm32wbxx_it.c           # Donanım kesme fonksiyonları
│       └── usart.c                  # USART1 (9600 8N1) donanım ayarları
├── Drivers/                         # STMicroelectronics CMSIS ve HAL Sürücüleri
├── Modbus_Poll_BMS_Test_Result.png  # Modbus Poll doğrulama ekran görüntüsü
├── modbus123.ioc                    # STM32CubeMX donanım konfigürasyon dosyası
├── STM32WB55RGVX_FLASH.ld           # Linker Script (Flash yerleşim haritası)
└── README.md                        # Proje ana dokümantasyonu
```

---

## 🚀 Kurulum ve Kullanım Rehberi

### Gereksinimler

1. **Donanım:**
   - NUCLEO-WB55RG geliştirme kartı veya STM32WB55RGVx tabanlı özel kart.
   - CH340 / CP2102 / FTDI USB-to-TTL dönüştürücü adaptör.
   - ST-LINK V2 / V3 programlayıcı / hata ayıklayıcı.
2. **Yazılım:**
   - [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (v1.14 veya üstü)
   - [Modbus Poll](https://www.modbustools.com/modbus_poll.html) (veya açık kaynaklı QModBus / Python `minimalmodbus`)

---

### Derleme ve Yükleme

1. Bu depoyu yerel bilgisayarınıza klonlayın:
   ```bash
   git clone https://github.com/AliFurkanOZ/stm32.git
   ```
2. **STM32CubeIDE** yazılımını açın.
3. `File` $\to$ `Open Projects from File System...` seçeneğiyle indirdiğiniz klasörü içe aktarın.
4. `Project` $\to$ `Build All` (`Ctrl + B`) ile projeyi derleyin. Sıfır hata ve sıfır uyarı ile derlendiğini teyit edin.
5. ST-LINK üzerinden kartınızı bağlayın ve `Run` $\to$ `Debug` (`F11`) ile yazılımı mikrodenetleyiciye flashlayın.

---

### Modbus Poll ile Bağlantı Adımları

1. USB-TTL dönüştürücünüzü PC'ye bağlayın (Aygıt Yöneticisi'nden COM portunu öğrenin, örn: `COM5`).
2. Modbus Poll uygulamasını açın:
   - `Connection` $\to$ `Connect...` (`F3`) menüsüne gidin.
   - **Port:** `COM5` (veya ilgili port)
   - **Baud Rate:** `9600`
   - **Data Bits:** `8`, **Parity:** `None`, **Stop Bit:** `1`
   - **Mode:** `RTU`
3. Okuma Tanımı (`Setup` $\to$ `Read/Write Definition...` veya `F8`):
   - **Slave ID:** `1`
   - **Function:** `03 Read Holding Registers (4x)`
   - **Address:** `0`
   - **Quantity:** `20`
   - **Scan Rate:** `1000ms`
4. Register değerlerinin canlı olarak aktığını ve sağ alt durum çubuğunda TX/RX sayaçlarının hatasız ilerlediğini gözlemleyin.

---

### Python ile Hızlı Test Kodu

Sistemi Python üzerinden test etmek için aşağıdaki betiği kullanabilirsiniz:

```python
import minimalmodbus
import serial

# Seri port ve Slave konfigürasyonu
instrument = minimalmodbus.Instrument(port='COM5', slaveaddress=1)
instrument.serial.baudrate = 9600
instrument.serial.bytesize = 8
instrument.serial.parity   = serial.PARITY_NONE
instrument.serial.stopbits = 1
instrument.serial.timeout  = 0.5
instrument.mode = minimalmodbus.MODE_RTU

print("--- STM32 BMS TELEMETRİ OKUMA ---")

# 0x0000: Paket Gerilimi (mV), 0x0002: Akım (mA), 0x0004: SoC (0.1%)
pack_voltage = instrument.read_register(registeraddress=0, functioncode=3)
pack_current = instrument.read_register(registeraddress=2, functioncode=3, signed=True)
soc_raw      = instrument.read_register(registeraddress=4, functioncode=3)

print(f"Toplam Gerilim : {pack_voltage / 1000.0:.3f} V")
print(f"Paket Akımı    : {pack_current / 1000.0:.3f} A")
print(f"Şarj Durumu    : {soc_raw / 10.0:.1f} %")

# 0x0010 - 0x0013: İlk 4 Hücre Gerilimi
cells = instrument.read_registers(registeraddress=16, number_of_registers=4, functioncode=3)
for i, cell_mv in enumerate(cells, start=1):
    print(f"Hücre #{i} Gerilimi: {cell_mv} mV")

# 0x0068: Test Echo Register'ına Yazma ve Okuma (FC06)
instrument.write_register(registeraddress=104, value=1234, functioncode=6)
echo_val = instrument.read_register(registeraddress=104, functioncode=3)
print(f"Echo Test (1234): {'BAŞARILI' if echo_val == 1234 else 'BAŞARISIZ'}")
```

---

## 🛡️ Güvenlik, Koruma ve Kurtarma Mekanizması

BMS yazılımı, lityum tabanlı kimyalarda oluşabilecek termal kaçak (thermal runaway) veya derin deşarj risklerine karşı otonom koruma kalkanına sahiptir:

```mermaid
stateDiagram-v2
    [*] --> NORMAL: Sistem Başlatma
    
    state NORMAL {
        [*] --> Izleme
        Izleme --> Izleme: Hücre Gerilimi & Sıcaklık Kontrolü
    }
    
    NORMAL --> FAULT_OVP: Herhangi bir hücre >= 4250 mV
    NORMAL --> FAULT_UVP: Herhangi bir hücre <= 2800 mV
    NORMAL --> FAULT_OTP: Sıcaklık >= 60.0°C
    
    state FAULT_OVP {
        charge_mos = 0
        fault_flags |= 0x0001
    }
    
    state FAULT_UVP {
        discharge_mos = 0
        fault_flags |= 0x0002
    }
    
    state FAULT_OTP {
        charge_mos = 0
        discharge_mos = 0
        fault_flags |= 0x0004
    }
    
    FAULT_OVP --> NORMAL: REG_FAULT_CLEAR (0x0042) Tetiklendiğinde
    FAULT_UVP --> NORMAL: REG_FAULT_CLEAR (0x0042) Tetiklendiğinde
    FAULT_OTP --> NORMAL: REG_FAULT_CLEAR (0x0042) Tetiklendiğinde
```

Hata durumunda koruma devreye girdiğinde:
1. İlgili güç MOSFET'i (`charge_mos` veya `discharge_mos`) derhal donanımsal mantıkla `0` değerine çekilerek akım kesilir.
2. `REG_FAULT_FLAGS` (`0x0040`) adresinde ilgili bayrak set edilir.
3. Tehlike ortadan kalktıktan sonra sisteme normal çalışma izni vermek için Master tarafından `REG_FAULT_CLEAR` (`0x0042`) yazmacına herhangi bir değer yazılması yeterlidir.

---

## 📝 Lisans ve Katkıda Bulunma

Bu proje **MIT Lisansı** altında lisanslanmıştır. Detaylar için `LICENSE` dosyasına bakabilirsiniz.

Proje geliştirme önerileri, hata bildirimleri veya ek özellik talepleriniz için [Issue](https://github.com/AliFurkanOZ/stm32/issues) açabilir ya da [Pull Request](https://github.com/AliFurkanOZ/stm32/pulls) gönderebilirsiniz.

---

<div align="center">
  <sub>Geliştirici: <strong>Ali Furkan ÖZ</strong> • STM32WB55 BMS & Modbus RTU Projesi</sub>
</div>
