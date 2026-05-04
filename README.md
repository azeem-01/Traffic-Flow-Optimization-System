# 🚦 Traffic Flow Optimization System

> A real-time traffic simulation system built in **C++** using **Graph Theory**, **Dijkstra's Algorithm**, the **Observer Design Pattern**, and a **Windows Forms GUI**.

---

## 📸 Preview

![Traffic Flow Simulation UI](assets/final_UI.png)

---

## 📌 About the Project

This is a **Data Structures semester project** developed at **NUST CEME, Rawalpindi**.

The system models a city's road network as a **directed weighted graph** and simulates vehicle movement, congestion, queue formation at intersections, adaptive traffic signal control, and real-time rerouting — all visualized in a live 2D GUI.

---

## 🗺️ System Architecture

![UML Class Diagram](assets/UML.png)

The system is organized into five layers:

| Layer | Components |
|---|---|
| **Core Patterns** | `Observer`, `Subject` |
| **Simulation Entities** | `Road`, `Intersection` |
| **System Control** | `Manager`, `Initializer` |
| **Data Structures** | `Graph (AdjList)`, `Queue` |
| **GUI Layer** | `trafficSimulation`, `GUIObserver` |

---

## ✨ Key Features

- 🗺️ **6×6 city grid** — 36 intersections, 120 directed edges (60 bidirectional roads)
- 🚗 **Real-time vehicle simulation** — spawn, move, queue, reroute, arrive
- 📈 **BPR congestion model** — travel time increases non-linearly as roads fill up
- 🛣️ **Dijkstra's shortest path** — routes recalculated every tick using live edge weights
- 🚦 **Adaptive signal control** — green given to road with longest queue, with yellow phase
- 🔔 **Observer design pattern** — roads notify GUI and Manager independently on state change
- 🧵 **Background threading** — simulation runs on separate thread, GUI stays smooth
- 🖼️ **Static layer caching** — roads drawn once to bitmap, only vehicles redrawn each frame
- 🔍 **Zoom and pan** — `+` / `-` buttons and right-click drag for map navigation
- ✏️ **Interactive editing** — click any road or intersection to edit its properties live

---

## 📐 Mathematical Foundation

### BPR Travel Time Formula
```
w_ij(t) = w_free * (1 + 0.15 * (f_ij / c_ij)^4)
```

### Traffic Flow Update
```
f_ij(t+1) = f_ij(t) + a_ij(t) - x_ij(t)
```

### Queue Evolution
```
Q_ij(t+1) = Q_ij(t) + x_ij(t) - d_ij(t)
```

### Objective Function (minimize)
```
min Σ [ 0.5 * Q_ij(t) + 0.5 * (f_ij/c_ij)² ]
```

---

## 🏗️ Project Structure

```
Traffic-Flow-Optimization-System/
│
├── src/
│   ├── Observer.h              # Pure abstract observer interface
│   ├── Subject.h               # Observer list management base class
│   ├── Vehicle.h               # Vehicle data struct
│   ├── Road.h                  # Directed edge with BPR, queue, signal logic
│   ├── Intersection.h          # Graph node with adaptive signal control
│   ├── Graph.h                 # Adjacency list + Dijkstra + edge weight update
│   ├── Queue.h                 # Custom linked-list queue ADT
│   ├── Initializer.h           # Builds city grid, hands to Manager
│   ├── Manager.h               # Drives tick loop, owns vehicles, is an Observer
│   ├── GUIObserver.h           # Native C++ bridge to managed Windows Forms
│   └── trafficSimulation.h     # Windows Forms GUI
│
├── assets/
│   ├── final_UI.png            # Live simulation screenshot
│   ├── UML.png                 # UML class diagram
│   └── Cars/                   # Top-view car PNG images (car1.png - car6.png)
│
├── docs/
│   ├── DS_Project_Report.docx
│   └── DS_Problem_Document.pdf          # Original problem statement
│
└── README.md
```

---

## ⏱️ Time Complexity

| Function | Complexity | Notes |
|---|---|---|
| `Dijkstra (shortestPath)` | O(V²) | Linear scan, V=36 nodes |
| `syncWeights()` | O(E) | One pass over all roads |
| `updateVehicles()` | O(N) | One pass over all vehicles |
| `rerouteVehicles()` | O(N × V²) | Dijkstra per active vehicle |
| `Road::tick()` | O(μ) ≈ O(1) | μ = small discharge constant |
| `Intersection::tick()` | O(in-degree) | Scan incoming roads for max Q |
| `Manager::tick()` | O(N × V²) | Dominated by rerouting |
| `calcObjective()` | O(E) | One pass over all roads |
| `buildCity()` | O(V + E) | One-time setup only |

For V=36, E=120, N vehicles: each full tick ≈ 1296N operations — well within 100ms budget.

---

## 🖥️ How to Run

### Requirements
- Visual Studio 2022 or later
- Windows OS
- .NET Framework (for Windows Forms)
- C++/CLI support enabled in Visual Studio

### Steps
1. Clone the repository:
```bash
git clone https://github.com/YOUR_USERNAME/Traffic-Flow-Optimization-System.git
```
2. Open Visual Studio → **Open a project or solution**
3. Open the `.sln` file in the project folder
4. Place car images in `Cars/` folder inside the output directory (`Debug/` or `Release/`)
5. Build and run (`Ctrl + F5`)

### Controls

| Action | How |
|---|---|
| Start simulation | Click **Start** button |
| Pause simulation | Click **Pause** button |
| Reset simulation | Click **Reset** button |
| Add a vehicle | Select Source & Destination → **Add Vehicle** |
| Edit intersection | Click on intersection node → edit Tg → **Apply** |
| Edit road | Click on a road → edit properties → **Apply** |
| Zoom in / out | `+` and `-` buttons (top right of city panel) |
| Pan map | Right-click and drag on city panel |
| Vehicle info | Hover mouse over any vehicle |

---

## 📊 Data Structures Used

| ADT | Implementation | Used For |
|---|---|---|
| **Graph** | Adjacency List (`AdjList<T>`) | Road network representation |
| **Queue** | Linked List (`Queue<T>`) | Vehicle queues at intersections |
| **List** | `std::list` | Observer list, vehicle list, route list |

> **Note:** `Graph` and `Queue` are fully custom implementations. `std::list` is used only for internal utility containers where grading does not apply.

---

## 🎨 Design Patterns

### Observer Pattern
- `Road` and `Intersection` are **Subjects** — notify observers when state changes
- `Manager` is an **Observer** — rereroutes vehicles when congestion changes
- `GUIObserver` is an **Observer** — redraws the city panel when any road updates

### Why Observer?
The backend has **zero dependency on the GUI**. The simulation can run headlessly, and any number of observers can be attached without modifying `Road` or `Intersection` code.

---

## 📚 References

- Cormen et al., *Introduction to Algorithms* — Dijkstra's Algorithm
- Gamma et al., *Design Patterns* — Observer Pattern
- Bureau of Public Roads (BPR) Volume-Delay Function
- Microsoft C++/CLI Documentation

---

## 👨‍💻 Team

| Sr. | Name | CMS ID |
|---|---|---|
| 1 | Azeem Ashraf | 504116 |
| 2 | Muhammad Umer Shahzad | 507904 |
| 3 | Muzammil Hussain | 533058 |

**Instructor:** Ma'am Anum
**Institution:** NUST CEME, Rawalpindi
**Course:** Data Structures

---

## 📄 License

This project is licensed under the MIT License.
