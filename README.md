# FundamentalWeb

**Fundamental Stock Dashboard Web**

เว็บ Dashboard สำหรับแสดงข้อมูล Fundamental ของหุ้นไทย โดยใช้ข้อมูลจาก `fundamental_v4.csv` ซึ่งสร้างโดยระบบ **FundamentalUpdater_rev5 / Fundamental V4**

## 🌐 Live Demo

**Frontend:**
https://fundamentalweb-frontend.onrender.com/

> Frontend ใช้ React + Vite และเรียกข้อมูลผ่าน REST API ของ Backend

## 🏗️ Architecture

```text
┌──────────────────────────────┐
│ FundamentalUpdater_rev5      │
│ Price V4 + Fundamental V4    │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│ fundamental_v4.csv           │
│ Source of Truth               │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│ GitHub Repository             │
│ FundamentalWeb / data/        │
└──────────────┬───────────────┘
               │ 30 sec sync
               ▼
┌──────────────────────────────┐
│ C++ Backend                   │
│ Boost.Beast / REST API        │
│ Auto Reload                   │
└──────────────┬───────────────┘
               │
               ▼
┌──────────────────────────────┐
│ React + TypeScript + Vite     │
│ Fundamental Dashboard         │
└──────────────────────────────┘
```

## 📁 Project Structure

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

## 📊 Data Source

ไฟล์หลักของเว็บไซต์คือ:

```text
data/fundamental_v4.csv
```

Source of Truth ของ Fundamental V4 คือ:

```text
C:\Program Files\FundamentalUpdater_rev4\Data\Fundamental\fundamental_v4.csv
```

**สำคัญ:** เว็บไซต์ใช้ `fundamental_v4.csv` เป็นไฟล์หลัก และ **ไม่ใช้ `fundamental_869.csv` แทน**

## 🔄 Data Update Flow

```text
SET Data
   ↓
FundamentalUpdater_rev5
   ↓
Fundamental V4
   ↓
fundamental_v4.csv
   ↓
GitHub
   ↓
Backend GitHub Sync (30 sec)
   ↓
data/fundamental_v4.csv
   ↓
Backend Auto Reload
   ↓
/api/stocks
   ↓
React Dashboard
```

เมื่อ `fundamental_v4.csv` มีข้อมูลใหม่ ระบบสามารถส่งข้อมูลชุดใหม่ไปยัง Web ได้โดยไม่ต้องแก้ Frontend และไม่ต้องแก้ Price V4

## ⚙️ Backend

Backend พัฒนาด้วย **C++ + Boost.Beast**

- Bind: `0.0.0.0`
- Port: ใช้ environment variable `PORT`
- Local default: `8080`
- Auto Reload: เปิดใช้งาน
- GitHub CSV Sync: ทุก 30 วินาที

### API Endpoints

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

### Health Check

```text
GET /api/health
```

ใช้สำหรับตรวจว่า Backend ทำงานอยู่หรือไม่

### Stock Data

```text
GET /api/stocks/AOT
```

ใช้ตรวจข้อมูลหุ้นรายตัว เช่น AOT และเหมาะสำหรับตรวจสอบว่า Backend โหลด CSV เวอร์ชันล่าสุดแล้วหรือยัง

## ♻️ Automatic CSV Reload

Backend ตรวจสอบ `last_write_time` ของ:

```text
data/fundamental_v4.csv
```

เมื่อไฟล์มีการเปลี่ยนแปลง Backend จะ Reload ข้อมูลหุ้นใหม่โดยไม่จำเป็นต้อง Restart Server

## ☁️ Render Deployment

ระบบถูกออกแบบให้สามารถ Deploy Backend และ Frontend บน **Render** ได้

ข้อกำหนดสำคัญของ Backend:

```text
PORT = Render-provided environment variable
HOST = 0.0.0.0
```

Frontend ใช้ `/api/stocks` เป็น API path ดังนั้นการ Deploy ต้องกำหนด routing/proxy ให้ `/api/*` ส่งต่อไปยัง Backend

## 💻 Frontend

Frontend ใช้:

- React
- TypeScript
- Vite
- CSS

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

Frontend เรียกข้อมูลผ่าน:

```text
/api/stocks
```

## 🛠️ Local Development

Clone repository:

```bash
git clone https://github.com/Somyotthanimwas/FundamentalWeb.git
cd FundamentalWeb
```

Build Frontend:

```bash
cd frontend
npm install
npm run build
```

Build Backend ด้วย C++17 + Boost.Beast ตาม environment ของเครื่อง แล้วรัน:

```bash
./fundamental_backend
```

ตรวจสอบ:

```bash
curl http://localhost:8080/api/health
```

## 📡 CSV Sync / Watch Tools

เครื่องมือที่ใช้กับข้อมูล V4:

```text
tools/sync_v4_csv.sh
tools/watch_v4_csv.sh
tools/start_v4_watch.sh
```

หน้าที่หลักคือช่วยนำ `fundamental_v4.csv` จาก FundamentalUpdater ไปยังพื้นที่ข้อมูลของ FundamentalWeb และตรวจสอบการเปลี่ยนแปลงของไฟล์

## 🔍 Monitoring / Troubleshooting

ตรวจ Backend:

```bash
curl http://localhost:8080/api/health
```

ตรวจ AOT:

```bash
curl http://localhost:8080/api/stocks/AOT
```

ดู Backend Log:

```bash
tail -f backend/backend.log
```

ตรวจเวลาที่ CSV ถูกแก้ไข:

```bash
stat data/fundamental_v4.csv
```

หาก CSV มีข้อมูลใหม่ แต่ API ยังไม่เปลี่ยน:

1. ตรวจว่าไฟล์ `data/fundamental_v4.csv` มีเวลาแก้ไขล่าสุดจริง
2. ตรวจ Backend Log ว่าพบการเปลี่ยนแปลงหรือไม่
3. ทดสอบ `GET /api/stocks/AOT`
4. หาก API เปลี่ยนแล้วแต่หน้าเว็บไม่เปลี่ยน ให้ Hard Refresh ด้วย `Ctrl + F5`
5. ตรวจ routing ของ `/api/*` ระหว่าง Frontend และ Backend

## ⚠️ Important Rules

1. `fundamental_v4.csv` คือ Source CSV หลักของ Web
2. **ห้ามใช้ `fundamental_869.csv` แทน `fundamental_v4.csv`**
3. **ไม่แก้ Logic ของ Price V4 ที่ทำงานอยู่แล้วเพียงเพื่อแก้ Web Dashboard**
4. Backend ต้อง bind ที่ `0.0.0.0` เมื่อ Deploy บน Render
5. Backend ต้องใช้ `PORT` ที่ Render กำหนด
6. CSV ที่เปลี่ยนควรทำให้ Backend Reload โดยไม่ต้อง Restart
7. ข้อมูลบน Web ต้องอ้างอิงข้อมูลจาก Fundamental V4

## 📌 Current System

```text
Frontend       React + TypeScript + Vite
Backend        C++ + Boost.Beast
Data           fundamental_v4.csv
Stocks         ~869 symbols
Auto Reload    Enabled
GitHub Sync    Enabled
Sync Interval 30 seconds
Hosting        Render
```

## 🔗 Repository

https://github.com/Somyotthanimwas/FundamentalWeb

## 📜 License

This project is maintained by **Plaifa Engineering**.

Copyright © Plaifa Engineering
