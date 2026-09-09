# FundamentalWeb

**Fundamental Stock Dashboard Web**

เว็บ Dashboard สำหรับแสดงข้อมูล Fundamental ของหุ้นไทย โดยใช้ข้อมูลจาก `fundamental_v4.csv` ซึ่งสร้างโดยระบบ FundamentalUpdater_rev5 / Fundamental V4

## Architecture

```text
FundamentalUpdater_rev5
        |
        v
fundamental_v4.csv
        |
        v
GitHub
        |
        v
FundamentalWeb Backend
        |
        | REST API
        v
Frontend (React + Vite)
        |
        v
Fundamental Stock Dashboard
```

## Project Structure

```text
FundamentalWeb/
├── backend/
│   ├── main.cpp
│   ├── fundamental_backend
│   ├── backend.log
│   └── backend.pid
├── frontend/
│   ├── src/
│   │   ├── App.tsx
│   │   └── App.css
│   ├── package.json
│   └── vite.config.ts
├── data/
│   └── fundamental_v4.csv
├── tools/
│   ├── sync_v4_csv.sh
│   ├── watch_v4_csv.sh
│   └── start_v4_watch.sh
└── README.md
```

## Data Source

ข้อมูลหลักของเว็บไซต์คือ:

```text
data/fundamental_v4.csv
```

Source of Truth ของ Fundamental V4 คือ:

```text
C:\Program Files\FundamentalUpdater_rev4\Data\Fundamental\fundamental_v4.csv
```

เว็บไซต์ใช้ `fundamental_v4.csv` เป็นไฟล์หลัก และไม่ใช้ `fundamental_869.csv` แทนไฟล์หลัก

## Backend

Backend พัฒนาด้วย **C++ + Boost.Beast** และรับ port จาก environment variable `PORT` โดยค่าเริ่มต้นคือ `8080` และต้อง bind ที่ `0.0.0.0` สำหรับการ Deploy บน Render

### API

```text
GET  /api/health
GET  /api/stocks
GET  /api/stocks/{SYMBOL}
POST /api/reload
```

ตัวอย่าง:

```bash
curl http://localhost:8080/api/health
curl http://localhost:8080/api/stocks
curl http://localhost:8080/api/stocks/AOT
curl -X POST http://localhost:8080/api/reload
```

## Automatic CSV Reload

Backend ตรวจสอบ `last_write_time` ของ `data/fundamental_v4.csv` และ Reload ข้อมูลเมื่อไฟล์มีการเปลี่ยนแปลง โดยไม่จำเป็นต้อง Restart Server

## GitHub CSV Sync

Backend มีระบบ GitHub CSV Sync เพื่อตรวจสอบและดาวน์โหลด `fundamental_v4.csv` จาก GitHub เป็นระยะ ค่าเริ่มต้นคือทุก **30 วินาที** จากนั้น Backend จะตรวจพบการเปลี่ยนแปลงและ Reload ข้อมูล

```text
FundamentalUpdater
       |
       v
fundamental_v4.csv
       |
       v
GitHub
       |
       v
GitHub CSV Sync (30s)
       |
       v
data/fundamental_v4.csv
       |
       v
Auto Reload
       |
       v
/api/stocks
       |
       v
Frontend
```

## Frontend

Frontend ใช้ **React + TypeScript + Vite**

ติดตั้ง:

```bash
cd frontend
npm install
```

Development:

```bash
npm run dev
```

Production build:

```bash
npm run build
```

Preview:

```bash
npm run preview -- --host 0.0.0.0
```

Frontend เรียก API ผ่าน:

```text
/api/stocks
```

## Local Development

Clone repository:

```bash
git clone https://github.com/Somyotthanimwas/FundamentalWeb.git
cd FundamentalWeb
```

ติดตั้ง Frontend:

```bash
cd frontend
npm install
npm run build
```

Build Backend ตาม environment ของเครื่อง เช่น C++17 และ Boost.Beast แล้วรัน:

```bash
./fundamental_backend
```

## Data Update Workflow

```text
SET Data
   |
   v
FundamentalUpdater_rev5
   |
   v
Fundamental V4
   |
   v
fundamental_v4.csv
   |
   v
Sync to FundamentalWeb / GitHub
   |
   v
Render Backend
   |
   v
Auto Reload
   |
   v
React Frontend
```

## Important Rules

1. `fundamental_v4.csv` คือ Source CSV หลักของ Web
2. ไม่ใช้ `fundamental_869.csv` แทน `fundamental_v4.csv`
3. ไม่ควรแก้ Logic ของ Price V4 ที่ทำงานอยู่แล้วเพียงเพื่อแก้ Web Dashboard
4. Backend ต้อง bind ที่ `0.0.0.0` และใช้ `PORT` ที่ Render กำหนด
5. เมื่อ CSV เปลี่ยน Backend ควร Reload โดยไม่ต้อง Restart

## Monitoring / Troubleshooting

ตรวจ Backend:

```bash
curl http://localhost:8080/api/health
```

ตรวจ AOT:

```bash
curl http://localhost:8080/api/stocks/AOT
```

ดู Log:

```bash
tail -f backend/backend.log
```

ตรวจไฟล์ CSV:

```bash
stat data/fundamental_v4.csv
```

ถ้า API เปลี่ยนข้อมูลแล้ว แต่หน้าเว็บยังไม่เปลี่ยน ให้ลอง Hard Refresh ด้วย `Ctrl + F5` และตรวจ routing ของ `/api/*` ระหว่าง Frontend กับ Backend

## Current System

```text
Frontend       React + TypeScript + Vite
Backend        C++ + Boost.Beast
Data           fundamental_v4.csv
Stocks         ~869 symbols
Auto Reload    Enabled
GitHub Sync    Enabled
Sync Interval 30 seconds
```

## Repository

https://github.com/Somyotthanimwas/FundamentalWeb

## License

This project is maintained by **Plaifa Engineering**.

Copyright © Plaifa Engineering
