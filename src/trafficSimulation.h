#pragma once
#include "Initializer.h"
#include "Manager.h"
#include "Observer.h"
#include "GUIObserver.h"
#include <cmath>
#include <msclr/marshal_cppstd.h>
#include <vcclr.h>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#using <System.Threading.dll>

namespace SemesterProject {
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Collections::Generic;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Drawing::Drawing2D;
    using namespace System::Threading;

    /// <summary>
    /// Summary for trafficSimulation
    /// </summary>
    public ref class trafficSimulation : public System::Windows::Forms::Form
    {
    public:
        trafficSimulation(void)
        {
            InitializeComponent();
            vScrollBar1->BringToFront();
            hScrollBar1->BringToFront();
            //
            //TODO: Add the constructor code here
            //
            this->SetStyle(ControlStyles::AllPaintingInWmPaint |
                ControlStyles::UserPaint |
                ControlStyles::OptimizedDoubleBuffer, true);
            this->UpdateStyles();
            System::Reflection::PropertyInfo^ p = panel1->GetType()->GetProperty("DoubleBuffered", System::Reflection::BindingFlags::Instance | System::Reflection::BindingFlags::NonPublic);
            if (p != nullptr) p->SetValue(panel1, true, nullptr); 
            System::Reflection::PropertyInfo^ r = panel1->GetType()->GetProperty("ResizeRedraw", System::Reflection::BindingFlags::Instance | System::Reflection::BindingFlags::NonPublic);
            if (r != nullptr) r->SetValue(panel1, true, nullptr);
            initBackend();
        }
    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~trafficSimulation()
        {
            if (components) delete components;
            simRunning = false;
            if (simThread != nullptr && simThread->IsAlive)
                simThread->Join(500);
            delete manager;
            delete initializer;
            if (staticLayer != nullptr) delete staticLayer;
        }
        //backend state
    private: Initializer* initializer;
    private: Manager* manager;
    private: bool          simRunning;
    private: Road* selectedRoad;
    private: Intersection* selectedIntersection;
    private: float         vehicleSpeedMultiplier; //1=normal, 2=fast, etc
           //threading
    private: Thread^ simThread;
    private: Object^ simLock;
           //camera
    private: float camX;
    private: float camY;
    private: float zoom;
    private: bool  dragging;
    private: System::Drawing::Point lastMouse;
           //static layer cache â€” roads + grass, rebuilt only when needed
    private: Bitmap^ staticLayer;
    private: bool    staticBuilt;
           //node position cache for fast lookup
    private: Dictionary<int, PointF>^ nodeMap;
           //car images
    private: cli::array<Image^>^ carImages;
           //form controls
    private: System::Windows::Forms::Panel^ panel1;     //city drawing
    private: System::Windows::Forms::Panel^ panel2;     //side panel
    private: System::Windows::Forms::Button^ button1;    //start
    private: System::Windows::Forms::Button^ button2;    //pause
    private: System::Windows::Forms::Button^ button3;    //reset
    private: System::Windows::Forms::Label^ label1;     //"Tick:"
    private: System::Windows::Forms::Label^ label2;     //tick value
    private: System::Windows::Forms::Button^ button8;    //zoom in
    private: System::Windows::Forms::Button^ button9;    //zoom out

    private: System::Windows::Forms::GroupBox^ groupBox1;  //metrics
    private: System::Windows::Forms::Label^ label3;
    private: System::Windows::Forms::Label^ label4;
    private: System::Windows::Forms::Label^ label5;
    private: System::Windows::Forms::Label^ label6;
    private: System::Windows::Forms::Label^ label7;
    private: System::Windows::Forms::Label^ label8;
    private: System::Windows::Forms::Label^ label9;
    private: System::Windows::Forms::Label^ label10;
    private: System::Windows::Forms::Label^ label11;
    private: System::Windows::Forms::Label^ label12;

    private: System::Windows::Forms::GroupBox^ groupBox2;  //add vehicle
    private: System::Windows::Forms::Label^ label13;
    private: System::Windows::Forms::ComboBox^ comboBox1;
    private: System::Windows::Forms::Label^ label14;
    private: System::Windows::Forms::ComboBox^ comboBox2;
    private: System::Windows::Forms::Button^ button4;

    private: System::Windows::Forms::GroupBox^ groupBox3;  //edit intersection
    private: System::Windows::Forms::Label^ label15;
    private: System::Windows::Forms::Label^ label16;
    private: System::Windows::Forms::Label^ label17;
    private: System::Windows::Forms::TextBox^ textBox1;
    private: System::Windows::Forms::Button^ button5;

    private: System::Windows::Forms::GroupBox^ groupBox4;  //edit road
    private: System::Windows::Forms::Label^ label18;
    private: System::Windows::Forms::Label^ label19;
    private: System::Windows::Forms::TextBox^ textBox2;
    private: System::Windows::Forms::Label^ label20;
    private: System::Windows::Forms::TextBox^ textBox3;
    private: System::Windows::Forms::Label^ label21;
    private: System::Windows::Forms::TextBox^ textBox4;
    private: System::Windows::Forms::Label^ label22;
    private: System::Windows::Forms::TextBox^ textBox5;
    private: System::Windows::Forms::Button^ button6;
    private: System::Windows::Forms::VScrollBar^ vScrollBar1;
    private: System::Windows::Forms::HScrollBar^ hScrollBar1;

    private: System::Windows::Forms::GroupBox^ groupBox6;  //speed control
    private: System::Windows::Forms::Label^ label27;
    private: System::Windows::Forms::TrackBar^ trackBar1;
    private: System::Windows::Forms::Label^ label28;

    private: System::Windows::Forms::ToolTip^ toolTip1;

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
    private: System::ComponentModel::IContainer^ components;

           //backend init
    private: void initBackend()
    {
        initializer = new Initializer();
        manager = new Manager(*initializer);
        simRunning = false;
        selectedRoad = nullptr;
        selectedIntersection = nullptr;
        vehicleSpeedMultiplier = 1.0f;
        simLock = gcnew Object();
        camX = 0;
        camY = 0;
        zoom = 1.6f;
        dragging = false;
        staticBuilt = false;
        staticLayer = nullptr;
        nodeMap = gcnew Dictionary<int, PointF>();
        for (Intersection* i : initializer->getIntersections())
            nodeMap[i->getId()] = PointF(i->getX(), i->getY());
        float maxX = 0, maxY = 0; //comment
        for each (KeyValuePair<int, PointF> kv in nodeMap)
        {
            if (kv.Value.X > maxX) maxX = kv.Value.X;
            if (kv.Value.Y > maxY) maxY = kv.Value.Y;
        }
        camX = Math::Max(0.0f, (maxX - (panel1->Width / zoom)) * 0.5f);
        camY = Math::Max(0.0f, (maxY - (panel1->Height / zoom)) * 0.5f);
        for (Road* r : initializer->getRoads())
            r->addObserver(new GUIObserver(this));
        for (Intersection* i : initializer->getIntersections())
            i->addObserver(new GUIObserver(this));
        loadCarImages();
        populateDropdowns();
    }
           //builds static road+grass layer once â€” reuse every frame
    private: bool isSafeFromRoads(float x, float y, float margin)
    {
        for (Road* r : manager->getRoads())
        {
            PointF p1 = getNodePos(r->getFromNode());
            PointF p2 = getNodePos(r->getToNode());
            float l2 = (p2.X - p1.X)*(p2.X - p1.X) + (p2.Y - p1.Y)*(p2.Y - p1.Y);
            if (l2 == 0) continue;
            float t = ((x - p1.X) * (p2.X - p1.X) + (y - p1.Y) * (p2.Y - p1.Y)) / l2;
            t = Math::Max(0.0f, Math::Min(1.0f, t));
            float projX = p1.X + t * (p2.X - p1.X);
            float projY = p1.Y + t * (p2.Y - p1.Y);
            float dist2 = (x - projX)*(x - projX) + (y - projY)*(y - projY);
            if (dist2 < margin * margin)
                return false;
        }
        return true;
    }

    private: void drawScenery(Graphics^ g, float maxX, float maxY)
    {
        Random^ rand = gcnew Random(42); // Seed for consistent city layout
        const float SPACING = 60.0f; // Density of scenery checks
        const float SAFE_MARGIN = 25.0f; // Distance from roads

        for (float x = 20.0f; x < maxX + 50.0f; x += SPACING)
        {
            for (float y = 20.0f; y < maxY + 50.0f; y += SPACING)
            {
                float bx = x + (float)(rand->NextDouble() * 30 - 15);
                float by = y + (float)(rand->NextDouble() * 30 - 15);

                if (isSafeFromRoads(bx, by, SAFE_MARGIN))
                {
                    int type = rand->Next(100);
                    if (type < 25)
                    {
                        // Building
                        float bw = 25.0f + (float)rand->NextDouble() * 15.0f;
                        float bh = 40.0f + (float)rand->NextDouble() * 30.0f;
                        Color bc = Color::FromArgb(rand->Next(180, 220), rand->Next(120, 160), rand->Next(100, 140));
                        g->FillRectangle(gcnew SolidBrush(bc), bx - bw/2, by - bh/2, bw, bh);
                        SolidBrush^ winBrush = gcnew SolidBrush(Color::FromArgb(220, 240, 255));
                        for (float wy = by - bh/2 + 5.0f; wy < by + bh/2 - 10.0f; wy += 10.0f)
                            for (float wx = bx - bw/2 + 4.0f; wx < bx + bw/2 - 4.0f; wx += 8.0f)
                                g->FillRectangle(winBrush, wx, wy, 4.0f, 6.0f);
                    }
                    else if (type < 50)
                    {
                        // House
                        float hw = 25.0f + (float)rand->NextDouble() * 10.0f;
                        float hh = 20.0f + (float)rand->NextDouble() * 10.0f;
                        Color hc = Color::FromArgb(rand->Next(220, 250), rand->Next(210, 240), rand->Next(160, 200));
                        g->FillRectangle(gcnew SolidBrush(hc), bx - hw/2, by, hw, hh);
                        cli::array<PointF>^ roof = { PointF(bx - hw/2 - 4.0f, by), PointF(bx + hw/2 + 4.0f, by), PointF(bx, by - 12.0f) };
                        g->FillPolygon(gcnew SolidBrush(Color::IndianRed), roof);
                        g->FillRectangle(gcnew SolidBrush(Color::SaddleBrown), bx - 4.0f, by + hh - 10.0f, 8.0f, 10.0f);
                    }
                    else if (type < 85)
                    {
                        // Trees
                        for (int t = 0; t < 2; t++)
                        {
                            float tx = bx + (float)(rand->NextDouble() * 20 - 10);
                            float ty = by + (float)(rand->NextDouble() * 20 - 10);
                            g->FillRectangle(gcnew SolidBrush(Color::SaddleBrown), tx - 2.0f, ty, 4.0f, 12.0f);
                            g->FillEllipse(gcnew SolidBrush(Color::ForestGreen), tx - 8.0f, ty - 12.0f, 16.0f, 16.0f);
                            g->FillEllipse(gcnew SolidBrush(Color::LimeGreen), tx - 4.0f, ty - 14.0f, 10.0f, 10.0f);
                        }
                    }
                    else
                    {
                        // Pond
                        float pw = 30.0f + (float)rand->NextDouble() * 20.0f;
                        float ph = 20.0f + (float)rand->NextDouble() * 15.0f;
                        g->FillEllipse(gcnew SolidBrush(Color::SteelBlue), bx - pw/2.0f, by - ph/2.0f, pw, ph);
                        g->FillEllipse(gcnew SolidBrush(Color::LightSkyBlue), bx - pw/2.0f + 4.0f, by - ph/2.0f + 4.0f, pw - 8.0f, ph - 8.0f);
                    }
                }
            }
        }
    }

    private: void buildStaticLayer()
    {
        if (staticLayer != nullptr) delete staticLayer;
        float maxX = 0, maxY = 0; //comment
        for each (KeyValuePair<int, PointF> kv in nodeMap)
        {
            if (kv.Value.X > maxX) maxX = kv.Value.X;
            if (kv.Value.Y > maxY) maxY = kv.Value.Y;
        }
        int w = (int)(maxX + 80);
        int h = (int)(maxY + 80);
        staticLayer = gcnew Bitmap(w > 0 ? w : 870, h > 0 ? h : 760);
        Graphics^ g = Graphics::FromImage(staticLayer);
        g->SmoothingMode = SmoothingMode::AntiAlias;
        g->PixelOffsetMode = PixelOffsetMode::HighQuality;
        g->Clear(Color::FromArgb(60, 120, 40)); //grass
        drawScenery(g, maxX, maxY);
        drawRoads(g);
        drawGate(g);
        delete g;
        staticBuilt = true;
    }
    private: void loadCarImages()
    {
        carImages = gcnew cli::array<Image^>(6);
        for (int i = 0; i < 6; i++)
        {
            String^ path = "Cars\\car" + (i + 1).ToString() + ".png";
            if (System::IO::File::Exists(path))
                carImages[i] = Image::FromFile(path);
        }
    }
    private: void populateDropdowns()
    {
        comboBox1->Items->Clear();
        comboBox2->Items->Clear();
        for (Intersection* inter : initializer->getIntersections())
        {
            String^ name = gcnew String(inter->getName().c_str());
            comboBox1->Items->Add(name);
            comboBox2->Items->Add(name);
        }
        if (comboBox1->Items->Count > 0) comboBox1->SelectedIndex = 0;
        if (comboBox2->Items->Count > 1) comboBox2->SelectedIndex = 1;
    }
           //called by GUIObserver from any thread â€” safely marshals to UI thread
    public: void onBackendUpdate()
    {
        if (this->IsHandleCreated)
        {
            if (this->InvokeRequired)
                this->BeginInvoke(gcnew Action(this, &trafficSimulation::refreshUI));
            else
                refreshUI();
        }
    }
    private: void refreshUI()
    {
        updateMetricsLabels();
        panel1->Invalidate();
    }
           //simulation thread loop
    private: void simLoop()
    {
        while (simRunning)
        {
            Monitor::Enter(simLock);
            try
            {
                int ticks = (int)vehicleSpeedMultiplier;
                for (int i = 0; i < ticks; i++)
                    manager->tick();
            }
            finally { Monitor::Exit(simLock); }
            Thread::Sleep(100); //100ms per cycle
        }
    }

           //drawing
    private: void drawCity(Graphics^ g)
    {
        if (!staticBuilt) buildStaticLayer();
        g->DrawImage(staticLayer, 0, 0); //blit static layer
        drawIntersections(g);            //dynamic â€” signal colors change
        drawVehicles(g);                 //dynamic â€” positions change
    }
    private: void drawRoads(Graphics^ g)
    {
        const float ROAD_W = 20.0f;    //total road width (both lanes)
        Monitor::Enter(simLock);
        try
        {
            for (Road* r : manager->getRoads())
            {
                PointF p1 = getNodePos(r->getFromNode());
                PointF p2 = getNodePos(r->getToNode());
                float rho = r->getRho();
                //congestion color
                Color roadColor;
                if (rho >= 0.8f) roadColor = Color::FromArgb(180, 40, 40);
                else if (rho >= 0.5f) roadColor = Color::FromArgb(200, 140, 0);
                else                  roadColor = Color::FromArgb(50, 50, 60);
                Pen^ roadPen = gcnew Pen(roadColor, ROAD_W);
                roadPen->StartCap = LineCap::Flat;
                roadPen->EndCap = LineCap::Flat;
                g->DrawLine(roadPen, p1, p2);
                //road borders
                float dx = p2.X - p1.X, dy = p2.Y - p1.Y;
                float len = (float)Math::Sqrt(dx * dx + dy * dy);
                if (len < 1.0f) continue;
                float nx = -dy / len * (ROAD_W / 2 + 1);
                float ny = dx / len * (ROAD_W / 2 + 1);
                Pen^ border = gcnew Pen(Color::FromArgb(25, 25, 30), 1.5f);
                g->DrawLine(border, p1.X + nx, p1.Y + ny, p2.X + nx, p2.Y + ny);
                g->DrawLine(border, p1.X - nx, p1.Y - ny, p2.X - nx, p2.Y - ny);
                //center dashed divider
                Pen^ dash = gcnew Pen(Color::FromArgb(220, 220, 220), 1.0f);
                dash->DashStyle = DashStyle::Dash;
                g->DrawLine(dash, p1, p2);
            }
        }
        finally { Monitor::Exit(simLock); }
    }
    private: void drawIntersections(Graphics^ g)
    {
        const float ROAD_W = 20.0f;
        const float HALF = 18.0f;
        const float LIGHT_BOX = 10.0f;
        const float LIGHT_R = 3.5f;
        Monitor::Enter(simLock);
        try
        {
            for (Intersection* inter : manager->getIntersections())
            {
                float cx = inter->getX();
                float cy = inter->getY();
                g->FillRectangle(gcnew SolidBrush(Color::FromArgb(40, 40, 50)),
                    RectangleF(cx - HALF, cy - HALF, HALF * 2, HALF * 2));
                g->DrawRectangle(gcnew Pen(Color::FromArgb(20, 20, 25), 1.5f), cx - HALF, cy - HALF, HALF * 2, HALF * 2);
                for (Road* r : inter->getInRoads())
                {
                    PointF p1 = getNodePos(r->getFromNode());
                    PointF p2 = getNodePos(r->getToNode());
                    float dx = p2.X - p1.X, dy = p2.Y - p1.Y;
                    float len = (float)Math::Sqrt(dx * dx + dy * dy);
                    if (len < 1.0f) continue;
                    float ux = dx / len, uy = dy / len;
                    float px = -uy, py = ux;
                    float offset = HALF + 6.0f;
                    float side = ROAD_W * 0.25f;
                    float lx = p2.X - ux * offset + px * side;
                    float ly = p2.Y - uy * offset + py * side;
                    bool isYellow = (r->getGTimer() > 0);
                    bool isGreen = (r->getG() == 1 && !isYellow);
                    Color redC = (!isGreen && !isYellow) ? Color::Red : Color::FromArgb(80, 25, 25);
                    Color yellowC = isYellow ? Color::Yellow : Color::FromArgb(80, 80, 25);
                    Color greenC = isGreen ? Color::Lime : Color::FromArgb(25, 80, 25);
                    g->FillRectangle(Brushes::Black, RectangleF(lx - LIGHT_BOX / 2, ly - LIGHT_BOX / 2, LIGHT_BOX, LIGHT_BOX * 2.4f));
                    g->FillEllipse(gcnew SolidBrush(redC), RectangleF(lx - LIGHT_R, ly - LIGHT_BOX * 0.4f - LIGHT_R, LIGHT_R * 2, LIGHT_R * 2));
                    g->FillEllipse(gcnew SolidBrush(yellowC), RectangleF(lx - LIGHT_R, ly - LIGHT_R, LIGHT_R * 2, LIGHT_R * 2));
                    g->FillEllipse(gcnew SolidBrush(greenC), RectangleF(lx - LIGHT_R, ly + LIGHT_BOX * 0.4f - LIGHT_R, LIGHT_R * 2, LIGHT_R * 2));
                }
                if (inter == selectedIntersection)
                    g->DrawRectangle(gcnew Pen(Color::Yellow, 2), cx - HALF, cy - HALF, HALF * 2, HALF * 2);
                g->DrawString(gcnew String(inter->getName().c_str()),
                    gcnew Drawing::Font("Arial", 6), Brushes::White, cx - 16, cy - HALF - 10);
            }
        }
        finally { Monitor::Exit(simLock); }
    }
    private: void drawVehicles(Graphics^ g)
    {
        const float ROAD_W = 20.0f;
        const float LANE_OFF = ROAD_W * 0.25f;
        float scaleFactor = (zoom > 1.0f) ? (1.0f + (zoom - 1.0f) * 0.5f) : 1.0f;
        float CAR_L = 26.0f * scaleFactor;
        float CAR_W = 14.0f * scaleFactor;
        const float QUEUE_GAP = 4.0f;
        Monitor::Enter(simLock);
        try
        {
            for (Vehicle* v : manager->getVehicles())
            {
                if (v->status == "arrived") continue;
                if (v->currEdge == nullptr) continue;
                PointF p1 = getNodePos(v->currEdge->getFromNode());
                PointF p2 = getNodePos(v->currEdge->getToNode());
                float dx = p2.X - p1.X, dy = p2.Y - p1.Y;
                float len = (float)Math::Sqrt(dx * dx + dy * dy);
                if (len < 1.0f) continue;
                float lox = -dy / len * LANE_OFF;
                float loy = dx / len * LANE_OFF;
                float progress = 0.0f;
                if (v->status == "waiting")
                {
                    int idx = 0;
                    for (Vehicle* qv : v->currEdge->getQueueList())
                    {
                        if (qv == v) break;
                        idx++;
                    }
                    float back = (idx + 1) * (26.0f + QUEUE_GAP); // Use constant logical length
                    progress = Math::Max(0.0f, Math::Min(1.0f, 1.0f - (back / len)));
                }
                else
                {
                    float total = v->currEdge->getW(); // Use current travel time instead of free-flow
                    progress = (total > 0) ? (1.0f - v->rv / total) : 0.0f;
                    progress = Math::Max(0.0f, Math::Min(1.0f, progress));
                }
                float vx = p1.X + dx * progress + lox;
                float vy = p1.Y + dy * progress + loy;

                if (v->route.size() > 0)
                {
                    Pen^ pathPen = gcnew Pen(Color::FromArgb(150, 255, 50, 50), 2.5f);
                    pathPen->DashStyle = DashStyle::Dot;
                    auto it = v->route.begin();
                    it++; 
                    PointF prevPos = PointF(vx, vy);
                    while (it != v->route.end())
                    {
                        PointF nextPos = getNodePos(*it);
                        g->DrawLine(pathPen, prevPos, nextPos);
                        prevPos = nextPos;
                        it++;
                    }
                }

                float angle = (float)(Math::Atan2(dy, dx) * 180.0 / Math::PI);
                int carIdx = v->id % 6;
                if (carImages != nullptr && carIdx < carImages->Length && carImages[carIdx] != nullptr)
                {
                    Matrix^ saved = g->Transform->Clone();
                    g->TranslateTransform(vx, vy, MatrixOrder::Prepend);
                    g->RotateTransform(angle, MatrixOrder::Prepend);
                    g->DrawImage(carImages[carIdx], -CAR_L / 2.0f, -CAR_W / 2.0f, CAR_L, CAR_W);
                    g->Transform = saved;
                    delete saved;
                }
                else
                {
                    Color c = (v->status == "waiting") ? Color::DeepSkyBlue : Color::Yellow;
                    g->FillEllipse(gcnew SolidBrush(c), vx - CAR_W / 2, vy - CAR_W / 2, CAR_W, CAR_W);
                }
            }
        }
        finally { Monitor::Exit(simLock); }
    }
    private: void drawGate(Graphics^ g)
    {
        int w = staticLayer->Width;
        int h = staticLayer->Height;
        Pen^ wall = gcnew Pen(Color::FromArgb(100, 70, 30), 10);
        g->DrawRectangle(wall, 5, 5, w - 10, h - 10);
        SolidBrush^ post = gcnew SolidBrush(Color::FromArgb(80, 50, 20));
        g->FillRectangle(post, 0, 0, 16, 16);
        g->FillRectangle(post, w - 16, 0, 16, 16);
        g->FillRectangle(post, 0, h - 16, 16, 16);
        g->FillRectangle(post, w - 16, h - 16, 16, 16);
    }
           //returns raw node position (for static layer drawing)
    private: PointF getNodePos(int id)
    {
        if (nodeMap->ContainsKey(id)) return nodeMap[id];
        return PointF(0, 0);
    }
           //returns zoom+pan adjusted position (for dynamic drawing)
    private: PointF getNodePosZoomed(int id)
    {
        PointF p = getNodePos(id);
        return PointF(p.X * zoom - camX, p.Y * zoom - camY);
    }
    private: void updateMetricsLabels()
    {
        label2->Text = manager->getTickCount().ToString();
        label4->Text = manager->getAvgW().ToString("F2");
        label6->Text = manager->getTotalDelay().ToString("F2");
        label8->Text = manager->getThroughput().ToString();
        label10->Text = manager->getAvgRho().ToString("F2");
        label12->Text = manager->getObjValue().ToString("F2");
    }
           //distance from point to line segment for road click detection
    private: float distToLine(float px, float py, float x1, float y1, float x2, float y2)
    {
        float dx = x2 - x1, dy = y2 - y1;
        float len2 = dx * dx + dy * dy;
        if (len2 == 0) return (float)Math::Sqrt((px - x1) * (px - x1) + (py - y1) * (py - y1));
        float t = Math::Max(0.0f, Math::Min(1.0f, ((px - x1) * dx + (py - y1) * dy) / len2));
        float projX = x1 + t * dx, projY = y1 + t * dy;
        return (float)Math::Sqrt((px - projX) * (px - projX) + (py - projY) * (py - projY));
    }

    private: System::Void vScrollBar1_Scroll(System::Object^ sender, System::Windows::Forms::ScrollEventArgs^ e) {
        camY = (float)vScrollBar1->Value;
        panel1->Invalidate();
    }
    private: System::Void hScrollBar1_Scroll(System::Object^ sender, System::Windows::Forms::ScrollEventArgs^ e) {
        camX = (float)hScrollBar1->Value;
        panel1->Invalidate();
    }
    private: void updateScrollBars() {
        if (vScrollBar1 == nullptr || hScrollBar1 == nullptr) return;
        float maxX = 1400.0f, maxY = 1200.0f; // Default city size
        float viewW = panel1->Width / zoom;
        float viewH = panel1->Height / zoom;
        hScrollBar1->Maximum = (int)maxX;
        hScrollBar1->LargeChange = (int)viewW;
        hScrollBar1->Value = (int)Math::Max(0.0f, Math::Min((float)hScrollBar1->Maximum - hScrollBar1->LargeChange, camX));
        vScrollBar1->Maximum = (int)maxY;
        vScrollBar1->LargeChange = (int)viewH;
        vScrollBar1->Value = (int)Math::Max(0.0f, Math::Min((float)vScrollBar1->Maximum - vScrollBar1->LargeChange, camY));
    }
           //event handlers
    private: System::Void trafficSimulation_Load(System::Object^ sender, System::EventArgs^ e)
    {
        updateScrollBars();
        panel1->Invalidate();
    }
    private: System::Void panel1_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e)
    {
        e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;
        e->Graphics->PixelOffsetMode = PixelOffsetMode::HighQuality;
        e->Graphics->TranslateTransform(-camX, -camY, MatrixOrder::Append);
        e->Graphics->ScaleTransform(zoom, zoom, MatrixOrder::Append);
        drawCity(e->Graphics);
    }
    private: System::Void panel1_MouseClick(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
    {
        //convert screen coords to world coords
        float wx = (e->X / zoom) + camX;
        float wy = (e->Y / zoom) + camY;
        selectedRoad = nullptr;
        selectedIntersection = nullptr;
        //check intersection click
        for (Intersection* inter : manager->getIntersections())
        {
            float dx = wx - inter->getX();
            float dy = wy - inter->getY();
            if (Math::Sqrt(dx * dx + dy * dy) < 20)
            {
                selectedIntersection = inter;
                label16->Text = gcnew String(inter->getName().c_str());
                textBox1->Text = inter->getTg().ToString();
                groupBox3->Visible = true;
                groupBox4->Visible = false;
                panel1->Invalidate();
                return;
            }
        }
        //check road click
        for (Road* r : manager->getRoads())
        {
            PointF p1 = getNodePos(r->getFromNode());
            PointF p2 = getNodePos(r->getToNode());
            if (distToLine(wx, wy, p1.X, p1.Y, p2.X, p2.Y) < 10)
            {
                selectedRoad = r;
                label18->Text = String::Format(gcnew String("Road: {0}->{1}"),
                    r->getFromNode(), r->getToNode());
                groupBox4->Visible = true;
                groupBox3->Visible = false;
                panel1->Invalidate();
                return;
            }
        }
        groupBox3->Visible = false;
        groupBox4->Visible = false;
        panel1->Invalidate();
    }
    private: System::Void panel1_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
    {
        if (e->Button == System::Windows::Forms::MouseButtons::Right)
        {
            dragging = true;
            lastMouse = e->Location;
            panel1->Cursor = Cursors::SizeAll;
        }
    }
    private: System::Void panel1_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
    {
        dragging = false;
        panel1->Cursor = Cursors::Default;
    }
    private: System::Void panel1_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
    {
        if (dragging)
        {
            camX -= (e->X - lastMouse.X) / zoom;
            camY -= (e->Y - lastMouse.Y) / zoom;
            lastMouse = e->Location;
            updateScrollBars();
            panel1->Invalidate();
            return;
        }
        //vehicle hover tooltip
        const float ROAD_W = 20.0f;
        const float LANE_OFF = ROAD_W * 0.25f;
        const float CAR_L = 26.0f;
        const float QUEUE_GAP = 4.0f;
        float wx = (e->X / zoom) + camX;
        float wy = (e->Y / zoom) + camY;
        for (Vehicle* v : manager->getVehicles())
        {
            if (v->status == "arrived" || v->currEdge == nullptr) continue;
            PointF p1 = getNodePos(v->currEdge->getFromNode());
            PointF p2 = getNodePos(v->currEdge->getToNode());
            float dx = p2.X - p1.X, dy = p2.Y - p1.Y;
            float len = (float)Math::Sqrt(dx * dx + dy * dy);
            if (len < 1.0f) continue;
            float lox = -dy / len * LANE_OFF;
            float loy = dx / len * LANE_OFF;
            float progress = 0.0f;
            if (v->status == "waiting")
            {
                int idx = 0;
                for (Vehicle* qv : v->currEdge->getQueueList())
                {
                    if (qv == v) break;
                    idx++;
                }
                float back = (idx + 1) * (CAR_L + QUEUE_GAP);
                progress = Math::Max(0.0f, Math::Min(1.0f, 1.0f - (back / len)));
            }
            else
            {
                float total = v->currEdge->getWFree();
                progress = (total > 0) ? 1.0f - v->rv / total : 0.0f;
            }
            float vx = p1.X + dx * progress + lox;
            float vy = p1.Y + dy * progress + loy;
            if (Math::Abs(wx - vx) < 10 && Math::Abs(wy - vy) < 10)
            {
                String^ info = String::Format(gcnew String("ID:{0} | {1} | Src:{2} Dst:{3} | Rv:{4:F1}"),
                    v->id, gcnew String(v->status.c_str()), v->src, v->dst, v->rv);
                toolTip1->SetToolTip(panel1, info);
                return;
            }
        }
        toolTip1->SetToolTip(panel1, "");
    }
    private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e)  //start
    {
        if (!simRunning)
        {
            simRunning = true;
            button1->Enabled = false;
            button2->Enabled = true;
            simThread = gcnew Thread(gcnew ThreadStart(this, &trafficSimulation::simLoop));
            simThread->IsBackground = true;
            simThread->Start();
        }
    }
    private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e)  //pause
    {
        simRunning = false;
        button1->Enabled = true;
        button2->Enabled = false;
    }
    private: System::Void button3_Click(System::Object^ sender, System::EventArgs^ e)  //reset
    {
        simRunning = false;
        if (simThread != nullptr && simThread->IsAlive)
            simThread->Join(500);
        Monitor::Enter(simLock);
        try
        {
            delete manager;
            delete initializer;
            initializer = new Initializer();
            manager = new Manager(*initializer);
            for (Road* r : initializer->getRoads())
                r->addObserver(new GUIObserver(this));
            for (Intersection* i : initializer->getIntersections())
                i->addObserver(new GUIObserver(this));
            nodeMap->Clear();
            for (Intersection* i : initializer->getIntersections())
                nodeMap[i->getId()] = PointF(i->getX(), i->getY());
            staticBuilt = false;
        }
        finally { Monitor::Exit(simLock); }
        populateDropdowns();
        button1->Enabled = true;
        button2->Enabled = false;
        label2->Text = gcnew String("0");
        panel1->Invalidate();
    }
    private: System::Void button4_Click(System::Object^ sender, System::EventArgs^ e)  //add vehicle
    {
        int src = comboBox1->SelectedIndex + 1;
        int dst = comboBox2->SelectedIndex + 1;
        if (src == dst) return;
        Monitor::Enter(simLock);
        try { manager->addVehicle(src, dst); }
        finally { Monitor::Exit(simLock); }
        panel1->Invalidate();
    }
    private: System::Void button5_Click(System::Object^ sender, System::EventArgs^ e)  //apply intersection edit
    {
        if (selectedIntersection == nullptr) return;
        try
        {
            float newTg = float::Parse(textBox1->Text);
            initializer->editIntersection(selectedIntersection->getId(), newTg);
            staticBuilt = false;
        }
        catch (...) { MessageBox::Show("Invalid Tg value."); }
    }
    private: System::Void button6_Click(System::Object^ sender, System::EventArgs^ e)  //apply road edit
    {
        if (selectedRoad == nullptr) return;
        try
        {
            float len = float::Parse(textBox2->Text);
            float spd = float::Parse(textBox3->Text);
            int   cap = int::Parse(textBox4->Text);
            float dis = float::Parse(textBox5->Text);
            initializer->editRoad(selectedRoad->getFromNode(), selectedRoad->getToNode(),
                len, spd, cap, dis);
            staticBuilt = false;
        }
        catch (...) { MessageBox::Show("Invalid road values."); }
    }
    private: System::Void button8_Click(System::Object^ sender, System::EventArgs^ e)  //zoom in
    {
        zoom = Math::Min(zoom + 0.1f, 3.0f);
        updateScrollBars();
        panel1->Invalidate();
    }
    private: System::Void button9_Click(System::Object^ sender, System::EventArgs^ e)  //zoom out
    {
        zoom = Math::Max(zoom - 0.1f, 0.5f);
        updateScrollBars();
        panel1->Invalidate();
    }
    private: System::Void trackBar1_Scroll(System::Object^ sender, System::EventArgs^ e)
    {
        vehicleSpeedMultiplier = (float)trackBar1->Value;
        label28->Text = "x" + trackBar1->Value.ToString();
    }

#pragma region Windows Form Designer generated code
           /// <summary>
           /// Required method for Designer support - do not modify
           /// the contents of this method with the code editor.
           /// </summary>
           void InitializeComponent(void)
           {
               this->components = (gcnew System::ComponentModel::Container());
               this->panel1 = (gcnew System::Windows::Forms::Panel());
               this->button8 = (gcnew System::Windows::Forms::Button());
               this->button9 = (gcnew System::Windows::Forms::Button());
               this->panel2 = (gcnew System::Windows::Forms::Panel());
               this->button1 = (gcnew System::Windows::Forms::Button());
               this->button2 = (gcnew System::Windows::Forms::Button());
               this->button3 = (gcnew System::Windows::Forms::Button());
               this->label1 = (gcnew System::Windows::Forms::Label());
               this->label2 = (gcnew System::Windows::Forms::Label());
               this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
               this->label18 = (gcnew System::Windows::Forms::Label());
               this->label19 = (gcnew System::Windows::Forms::Label());
               this->textBox2 = (gcnew System::Windows::Forms::TextBox());
               this->label20 = (gcnew System::Windows::Forms::Label());
               this->textBox3 = (gcnew System::Windows::Forms::TextBox());
               this->label21 = (gcnew System::Windows::Forms::Label());
               this->textBox4 = (gcnew System::Windows::Forms::TextBox());
               this->label22 = (gcnew System::Windows::Forms::Label());
               this->textBox5 = (gcnew System::Windows::Forms::TextBox());
               this->button6 = (gcnew System::Windows::Forms::Button());
               this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
               this->label3 = (gcnew System::Windows::Forms::Label());
               this->label4 = (gcnew System::Windows::Forms::Label());
               this->label5 = (gcnew System::Windows::Forms::Label());
               this->label6 = (gcnew System::Windows::Forms::Label());
               this->label7 = (gcnew System::Windows::Forms::Label());
               this->label8 = (gcnew System::Windows::Forms::Label());
               this->label9 = (gcnew System::Windows::Forms::Label());
               this->label10 = (gcnew System::Windows::Forms::Label());
               this->label11 = (gcnew System::Windows::Forms::Label());
               this->label12 = (gcnew System::Windows::Forms::Label());
               this->groupBox6 = (gcnew System::Windows::Forms::GroupBox());
               this->label27 = (gcnew System::Windows::Forms::Label());
               this->trackBar1 = (gcnew System::Windows::Forms::TrackBar());
               this->label28 = (gcnew System::Windows::Forms::Label());
               this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
               this->label13 = (gcnew System::Windows::Forms::Label());
               this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
               this->label14 = (gcnew System::Windows::Forms::Label());
               this->comboBox2 = (gcnew System::Windows::Forms::ComboBox());
               this->button4 = (gcnew System::Windows::Forms::Button());
               this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
               this->label15 = (gcnew System::Windows::Forms::Label());
               this->label16 = (gcnew System::Windows::Forms::Label());
               this->label17 = (gcnew System::Windows::Forms::Label());
               this->textBox1 = (gcnew System::Windows::Forms::TextBox());
               this->button5 = (gcnew System::Windows::Forms::Button());
               this->vScrollBar1 = (gcnew System::Windows::Forms::VScrollBar());
               this->hScrollBar1 = (gcnew System::Windows::Forms::HScrollBar());
               this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
               this->panel1->SuspendLayout();
               this->panel2->SuspendLayout();
               this->groupBox4->SuspendLayout();
               this->groupBox1->SuspendLayout();
               this->groupBox6->SuspendLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar1))->BeginInit();
               this->groupBox2->SuspendLayout();
               this->groupBox3->SuspendLayout();
               this->SuspendLayout();
               // 
               // panel1
               // 
               this->panel1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(60)), static_cast<System::Int32>(static_cast<System::Byte>(120)),
                   static_cast<System::Int32>(static_cast<System::Byte>(40)));
               this->panel1->Controls->Add(this->button8);
               this->panel1->Controls->Add(this->button9);
               this->panel1->Location = System::Drawing::Point(0, 0);
               this->panel1->Name = L"panel1";
               this->panel1->Size = System::Drawing::Size(845, 605);
               this->panel1->TabIndex = 0;
               this->panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &trafficSimulation::panel1_Paint);
               this->panel1->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &trafficSimulation::panel1_MouseClick);
               this->panel1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &trafficSimulation::panel1_MouseDown);
               this->panel1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &trafficSimulation::panel1_MouseMove);
               this->panel1->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &trafficSimulation::panel1_MouseUp);
               // 
               // button8
               // 
               this->button8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
               this->button8->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(60)), static_cast<System::Int32>(static_cast<System::Byte>(60)),
                   static_cast<System::Int32>(static_cast<System::Byte>(80)));
               this->button8->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button8->ForeColor = System::Drawing::Color::White;
               this->button8->Location = System::Drawing::Point(767, 12);
               this->button8->Name = L"button8";
               this->button8->Size = System::Drawing::Size(30, 28);
               this->button8->TabIndex = 3;
               this->button8->Text = L"+";
               this->button8->UseVisualStyleBackColor = false;
               this->button8->Click += gcnew System::EventHandler(this, &trafficSimulation::button8_Click);
               // 
               // button9
               // 
               this->button9->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
               this->button9->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(60)), static_cast<System::Int32>(static_cast<System::Byte>(60)),
                   static_cast<System::Int32>(static_cast<System::Byte>(80)));
               this->button9->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button9->ForeColor = System::Drawing::Color::White;
               this->button9->Location = System::Drawing::Point(803, 12);
               this->button9->Name = L"button9";
               this->button9->Size = System::Drawing::Size(33, 28);
               this->button9->TabIndex = 4;
               this->button9->Text = L"-";
               this->button9->UseVisualStyleBackColor = false;
               this->button9->Click += gcnew System::EventHandler(this, &trafficSimulation::button9_Click);
               // 
               // panel2
               // 
               this->panel2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
                   static_cast<System::Int32>(static_cast<System::Byte>(48)));
               this->panel2->Controls->Add(this->button1);
               this->panel2->Controls->Add(this->button2);
               this->panel2->Controls->Add(this->button3);
               this->panel2->Controls->Add(this->label1);
               this->panel2->Controls->Add(this->label2);
               this->panel2->Controls->Add(this->groupBox4);
               this->panel2->Controls->Add(this->groupBox1);
               this->panel2->Controls->Add(this->groupBox6);
               this->panel2->Controls->Add(this->groupBox2);
               this->panel2->Controls->Add(this->groupBox3);
               this->panel2->Location = System::Drawing::Point(875, 0);
               this->panel2->Name = L"panel2";
               this->panel2->Size = System::Drawing::Size(361, 630);
               this->panel2->TabIndex = 1;
               // 
               // button1
               // 
               this->button1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(150)),
                   static_cast<System::Int32>(static_cast<System::Byte>(0)));
               this->button1->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button1->ForeColor = System::Drawing::Color::White;
               this->button1->Location = System::Drawing::Point(5, 8);
               this->button1->Name = L"button1";
               this->button1->Size = System::Drawing::Size(70, 28);
               this->button1->TabIndex = 0;
               this->button1->Text = L"Start";
               this->button1->UseVisualStyleBackColor = false;
               this->button1->Click += gcnew System::EventHandler(this, &trafficSimulation::button1_Click);
               // 
               // button2
               // 
               this->button2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(140)),
                   static_cast<System::Int32>(static_cast<System::Byte>(0)));
               this->button2->Enabled = false;
               this->button2->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button2->ForeColor = System::Drawing::Color::White;
               this->button2->Location = System::Drawing::Point(80, 8);
               this->button2->Name = L"button2";
               this->button2->Size = System::Drawing::Size(70, 28);
               this->button2->TabIndex = 1;
               this->button2->Text = L"Pause";
               this->button2->UseVisualStyleBackColor = false;
               this->button2->Click += gcnew System::EventHandler(this, &trafficSimulation::button2_Click);
               // 
               // button3
               // 
               this->button3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(180)), static_cast<System::Int32>(static_cast<System::Byte>(0)),
                   static_cast<System::Int32>(static_cast<System::Byte>(0)));
               this->button3->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button3->ForeColor = System::Drawing::Color::White;
               this->button3->Location = System::Drawing::Point(155, 8);
               this->button3->Name = L"button3";
               this->button3->Size = System::Drawing::Size(70, 28);
               this->button3->TabIndex = 2;
               this->button3->Text = L"Reset";
               this->button3->UseVisualStyleBackColor = false;
               this->button3->Click += gcnew System::EventHandler(this, &trafficSimulation::button3_Click);
               // 
               // label1
               // 
               this->label1->AutoSize = true;
               this->label1->ForeColor = System::Drawing::Color::Silver;
               this->label1->Location = System::Drawing::Point(298, 14);
               this->label1->Name = L"label1";
               this->label1->Size = System::Drawing::Size(31, 13);
               this->label1->TabIndex = 5;
               this->label1->Text = L"Tick:";
               // 
               // label2
               // 
               this->label2->AutoSize = true;
               this->label2->ForeColor = System::Drawing::Color::Lime;
               this->label2->Location = System::Drawing::Point(329, 15);
               this->label2->Name = L"label2";
               this->label2->Size = System::Drawing::Size(13, 13);
               this->label2->TabIndex = 6;
               this->label2->Text = L"0";
               // 
               // groupBox4
               // 
               this->groupBox4->Controls->Add(this->label18);
               this->groupBox4->Controls->Add(this->label19);
               this->groupBox4->Controls->Add(this->textBox2);
               this->groupBox4->Controls->Add(this->label20);
               this->groupBox4->Controls->Add(this->textBox3);
               this->groupBox4->Controls->Add(this->label21);
               this->groupBox4->Controls->Add(this->textBox4);
               this->groupBox4->Controls->Add(this->label22);
               this->groupBox4->Controls->Add(this->textBox5);
               this->groupBox4->Controls->Add(this->button6);
               this->groupBox4->ForeColor = System::Drawing::Color::White;
               this->groupBox4->Location = System::Drawing::Point(5, 456);
               this->groupBox4->Name = L"groupBox4";
               this->groupBox4->Size = System::Drawing::Size(350, 175);
               this->groupBox4->TabIndex = 4;
               this->groupBox4->TabStop = false;
               this->groupBox4->Text = L"Edit Road";
               this->groupBox4->Visible = false;
               // 
               // label18
               // 
               this->label18->AutoSize = true;
               this->label18->ForeColor = System::Drawing::Color::Yellow;
               this->label18->Location = System::Drawing::Point(10, 18);
               this->label18->Name = L"label18";
               this->label18->Size = System::Drawing::Size(63, 13);
               this->label18->TabIndex = 0;
               this->label18->Text = L"Road: none";
               // 
               // label19
               // 
               this->label19->AutoSize = true;
               this->label19->ForeColor = System::Drawing::Color::Silver;
               this->label19->Location = System::Drawing::Point(10, 42);
               this->label19->Name = L"label19";
               this->label19->Size = System::Drawing::Size(43, 13);
               this->label19->TabIndex = 1;
               this->label19->Text = L"Length:";
               // 
               // textBox2
               // 
               this->textBox2->Location = System::Drawing::Point(120, 39);
               this->textBox2->Name = L"textBox2";
               this->textBox2->Size = System::Drawing::Size(215, 20);
               this->textBox2->TabIndex = 2;
               // 
               // label20
               // 
               this->label20->AutoSize = true;
               this->label20->ForeColor = System::Drawing::Color::Silver;
               this->label20->Location = System::Drawing::Point(10, 68);
               this->label20->Name = L"label20";
               this->label20->Size = System::Drawing::Size(64, 13);
               this->label20->TabIndex = 3;
               this->label20->Text = L"Max Speed:";
               // 
               // textBox3
               // 
               this->textBox3->Location = System::Drawing::Point(120, 65);
               this->textBox3->Name = L"textBox3";
               this->textBox3->Size = System::Drawing::Size(215, 20);
               this->textBox3->TabIndex = 4;
               // 
               // label21
               // 
               this->label21->AutoSize = true;
               this->label21->ForeColor = System::Drawing::Color::Silver;
               this->label21->Location = System::Drawing::Point(10, 94);
               this->label21->Name = L"label21";
               this->label21->Size = System::Drawing::Size(51, 13);
               this->label21->TabIndex = 5;
               this->label21->Text = L"Capacity:";
               // 
               // textBox4
               // 
               this->textBox4->Location = System::Drawing::Point(120, 91);
               this->textBox4->Name = L"textBox4";
               this->textBox4->Size = System::Drawing::Size(215, 20);
               this->textBox4->TabIndex = 6;
               // 
               // label22
               // 
               this->label22->AutoSize = true;
               this->label22->ForeColor = System::Drawing::Color::Silver;
               this->label22->Location = System::Drawing::Point(10, 120);
               this->label22->Name = L"label22";
               this->label22->Size = System::Drawing::Size(58, 13);
               this->label22->TabIndex = 7;
               this->label22->Text = L"Discharge:";
               // 
               // textBox5
               // 
               this->textBox5->Location = System::Drawing::Point(120, 117);
               this->textBox5->Name = L"textBox5";
               this->textBox5->Size = System::Drawing::Size(215, 20);
               this->textBox5->TabIndex = 8;
               // 
               // button6
               // 
               this->button6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(120)),
                   static_cast<System::Int32>(static_cast<System::Byte>(215)));
               this->button6->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button6->ForeColor = System::Drawing::Color::White;
               this->button6->Location = System::Drawing::Point(120, 148);
               this->button6->Name = L"button6";
               this->button6->Size = System::Drawing::Size(215, 24);
               this->button6->TabIndex = 9;
               this->button6->Text = L"Apply";
               this->button6->UseVisualStyleBackColor = false;
               this->button6->Click += gcnew System::EventHandler(this, &trafficSimulation::button6_Click);
               // 
               // groupBox1
               // 
               this->groupBox1->Controls->Add(this->label3);
               this->groupBox1->Controls->Add(this->label4);
               this->groupBox1->Controls->Add(this->label5);
               this->groupBox1->Controls->Add(this->label6);
               this->groupBox1->Controls->Add(this->label7);
               this->groupBox1->Controls->Add(this->label8);
               this->groupBox1->Controls->Add(this->label9);
               this->groupBox1->Controls->Add(this->label10);
               this->groupBox1->Controls->Add(this->label11);
               this->groupBox1->Controls->Add(this->label12);
               this->groupBox1->ForeColor = System::Drawing::Color::White;
               this->groupBox1->Location = System::Drawing::Point(5, 40);
               this->groupBox1->Name = L"groupBox1";
               this->groupBox1->Size = System::Drawing::Size(350, 150);
               this->groupBox1->TabIndex = 0;
               this->groupBox1->TabStop = false;
               this->groupBox1->Text = L"Performance Metrics";
               // 
               // label3
               // 
               this->label3->AutoSize = true;
               this->label3->ForeColor = System::Drawing::Color::Silver;
               this->label3->Location = System::Drawing::Point(10, 22);
               this->label3->Name = L"label3";
               this->label3->Size = System::Drawing::Size(88, 13);
               this->label3->TabIndex = 0;
               this->label3->Text = L"Avg Travel Time:";
               // 
               // label4
               // 
               this->label4->AutoSize = true;
               this->label4->ForeColor = System::Drawing::Color::Lime;
               this->label4->Location = System::Drawing::Point(240, 22);
               this->label4->Name = L"label4";
               this->label4->Size = System::Drawing::Size(28, 13);
               this->label4->TabIndex = 1;
               this->label4->Text = L"0.00";
               // 
               // label5
               // 
               this->label5->AutoSize = true;
               this->label5->ForeColor = System::Drawing::Color::Silver;
               this->label5->Location = System::Drawing::Point(10, 48);
               this->label5->Name = L"label5";
               this->label5->Size = System::Drawing::Size(64, 13);
               this->label5->TabIndex = 2;
               this->label5->Text = L"Total Delay:";
               // 
               // label6
               // 
               this->label6->AutoSize = true;
               this->label6->ForeColor = System::Drawing::Color::Lime;
               this->label6->Location = System::Drawing::Point(240, 48);
               this->label6->Name = L"label6";
               this->label6->Size = System::Drawing::Size(28, 13);
               this->label6->TabIndex = 3;
               this->label6->Text = L"0.00";
               // 
               // label7
               // 
               this->label7->AutoSize = true;
               this->label7->ForeColor = System::Drawing::Color::Silver;
               this->label7->Location = System::Drawing::Point(10, 74);
               this->label7->Name = L"label7";
               this->label7->Size = System::Drawing::Size(65, 13);
               this->label7->TabIndex = 4;
               this->label7->Text = L"Throughput:";
               // 
               // label8
               // 
               this->label8->AutoSize = true;
               this->label8->ForeColor = System::Drawing::Color::Lime;
               this->label8->Location = System::Drawing::Point(240, 74);
               this->label8->Name = L"label8";
               this->label8->Size = System::Drawing::Size(13, 13);
               this->label8->TabIndex = 5;
               this->label8->Text = L"0";
               // 
               // label9
               // 
               this->label9->AutoSize = true;
               this->label9->ForeColor = System::Drawing::Color::Silver;
               this->label9->Location = System::Drawing::Point(10, 100);
               this->label9->Name = L"label9";
               this->label9->Size = System::Drawing::Size(85, 13);
               this->label9->TabIndex = 6;
               this->label9->Text = L"Avg Congestion:";
               // 
               // label10
               // 
               this->label10->AutoSize = true;
               this->label10->ForeColor = System::Drawing::Color::Orange;
               this->label10->Location = System::Drawing::Point(240, 100);
               this->label10->Name = L"label10";
               this->label10->Size = System::Drawing::Size(28, 13);
               this->label10->TabIndex = 7;
               this->label10->Text = L"0.00";
               // 
               // label11
               // 
               this->label11->AutoSize = true;
               this->label11->ForeColor = System::Drawing::Color::Silver;
               this->label11->Location = System::Drawing::Point(10, 126);
               this->label11->Name = L"label11";
               this->label11->Size = System::Drawing::Size(85, 13);
               this->label11->TabIndex = 8;
               this->label11->Text = L"Objective Value:";
               // 
               // label12
               // 
               this->label12->AutoSize = true;
               this->label12->ForeColor = System::Drawing::Color::Cyan;
               this->label12->Location = System::Drawing::Point(240, 126);
               this->label12->Name = L"label12";
               this->label12->Size = System::Drawing::Size(28, 13);
               this->label12->TabIndex = 9;
               this->label12->Text = L"0.00";
               // 
               // groupBox6
               // 
               this->groupBox6->Controls->Add(this->label27);
               this->groupBox6->Controls->Add(this->trackBar1);
               this->groupBox6->Controls->Add(this->label28);
               this->groupBox6->ForeColor = System::Drawing::Color::White;
               this->groupBox6->Location = System::Drawing::Point(5, 196);
               this->groupBox6->Name = L"groupBox6";
               this->groupBox6->Size = System::Drawing::Size(350, 69);
               this->groupBox6->TabIndex = 1;
               this->groupBox6->TabStop = false;
               this->groupBox6->Text = L"Simulation Speed";
               // 
               // label27
               // 
               this->label27->AutoSize = true;
               this->label27->ForeColor = System::Drawing::Color::Silver;
               this->label27->Location = System::Drawing::Point(10, 28);
               this->label27->Name = L"label27";
               this->label27->Size = System::Drawing::Size(41, 13);
               this->label27->TabIndex = 0;
               this->label27->Text = L"Speed:";
               // 
               // trackBar1
               // 
               this->trackBar1->Location = System::Drawing::Point(60, 16);
               this->trackBar1->Maximum = 5;
               this->trackBar1->Minimum = 1;
               this->trackBar1->Name = L"trackBar1";
               this->trackBar1->Size = System::Drawing::Size(220, 45);
               this->trackBar1->TabIndex = 1;
               this->trackBar1->Value = 1;
               this->trackBar1->Scroll += gcnew System::EventHandler(this, &trafficSimulation::trackBar1_Scroll);
               // 
               // label28
               // 
               this->label28->AutoSize = true;
               this->label28->ForeColor = System::Drawing::Color::Lime;
               this->label28->Location = System::Drawing::Point(290, 28);
               this->label28->Name = L"label28";
               this->label28->Size = System::Drawing::Size(18, 13);
               this->label28->TabIndex = 2;
               this->label28->Text = L"x1";
               // 
               // groupBox2
               // 
               this->groupBox2->Controls->Add(this->label13);
               this->groupBox2->Controls->Add(this->comboBox1);
               this->groupBox2->Controls->Add(this->label14);
               this->groupBox2->Controls->Add(this->comboBox2);
               this->groupBox2->Controls->Add(this->button4);
               this->groupBox2->ForeColor = System::Drawing::Color::White;
               this->groupBox2->Location = System::Drawing::Point(5, 265);
               this->groupBox2->Name = L"groupBox2";
               this->groupBox2->Size = System::Drawing::Size(350, 105);
               this->groupBox2->TabIndex = 2;
               this->groupBox2->TabStop = false;
               this->groupBox2->Text = L"Add Vehicle";
               // 
               // label13
               // 
               this->label13->AutoSize = true;
               this->label13->ForeColor = System::Drawing::Color::Silver;
               this->label13->Location = System::Drawing::Point(10, 25);
               this->label13->Name = L"label13";
               this->label13->Size = System::Drawing::Size(44, 13);
               this->label13->TabIndex = 0;
               this->label13->Text = L"Source:";
               // 
               // comboBox1
               // 
               this->comboBox1->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
               this->comboBox1->Location = System::Drawing::Point(90, 22);
               this->comboBox1->Name = L"comboBox1";
               this->comboBox1->Size = System::Drawing::Size(250, 21);
               this->comboBox1->TabIndex = 1;
               // 
               // label14
               // 
               this->label14->AutoSize = true;
               this->label14->ForeColor = System::Drawing::Color::Silver;
               this->label14->Location = System::Drawing::Point(10, 52);
               this->label14->Name = L"label14";
               this->label14->Size = System::Drawing::Size(63, 13);
               this->label14->TabIndex = 2;
               this->label14->Text = L"Destination:";
               // 
               // comboBox2
               // 
               this->comboBox2->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
               this->comboBox2->Location = System::Drawing::Point(90, 49);
               this->comboBox2->Name = L"comboBox2";
               this->comboBox2->Size = System::Drawing::Size(250, 21);
               this->comboBox2->TabIndex = 3;
               // 
               // button4
               // 
               this->button4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(120)),
                   static_cast<System::Int32>(static_cast<System::Byte>(215)));
               this->button4->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button4->ForeColor = System::Drawing::Color::White;
               this->button4->Location = System::Drawing::Point(90, 76);
               this->button4->Name = L"button4";
               this->button4->Size = System::Drawing::Size(180, 24);
               this->button4->TabIndex = 4;
               this->button4->Text = L"Add Vehicle";
               this->button4->UseVisualStyleBackColor = false;
               this->button4->Click += gcnew System::EventHandler(this, &trafficSimulation::button4_Click);
               // 
               // groupBox3
               // 
               this->groupBox3->Controls->Add(this->label15);
               this->groupBox3->Controls->Add(this->label16);
               this->groupBox3->Controls->Add(this->label17);
               this->groupBox3->Controls->Add(this->textBox1);
               this->groupBox3->Controls->Add(this->button5);
               this->groupBox3->ForeColor = System::Drawing::Color::White;
               this->groupBox3->Location = System::Drawing::Point(5, 371);
               this->groupBox3->Name = L"groupBox3";
               this->groupBox3->Size = System::Drawing::Size(350, 79);
               this->groupBox3->TabIndex = 3;
               this->groupBox3->TabStop = false;
               this->groupBox3->Text = L"Edit Intersection";
               this->groupBox3->Visible = false;
               // 
               // label15
               // 
               this->label15->AutoSize = true;
               this->label15->ForeColor = System::Drawing::Color::Silver;
               this->label15->Location = System::Drawing::Point(10, 25);
               this->label15->Name = L"label15";
               this->label15->Size = System::Drawing::Size(52, 13);
               this->label15->TabIndex = 0;
               this->label15->Text = L"Selected:";
               // 
               // label16
               // 
               this->label16->AutoSize = true;
               this->label16->ForeColor = System::Drawing::Color::Yellow;
               this->label16->Location = System::Drawing::Point(80, 25);
               this->label16->Name = L"label16";
               this->label16->Size = System::Drawing::Size(33, 13);
               this->label16->TabIndex = 1;
               this->label16->Text = L"None";
               // 
               // label17
               // 
               this->label17->AutoSize = true;
               this->label17->ForeColor = System::Drawing::Color::Silver;
               this->label17->Location = System::Drawing::Point(10, 52);
               this->label17->Name = L"label17";
               this->label17->Size = System::Drawing::Size(23, 13);
               this->label17->TabIndex = 2;
               this->label17->Text = L"Tg:";
               // 
               // textBox1
               // 
               this->textBox1->Location = System::Drawing::Point(80, 49);
               this->textBox1->Name = L"textBox1";
               this->textBox1->Size = System::Drawing::Size(120, 20);
               this->textBox1->TabIndex = 3;
               // 
               // button5
               // 
               this->button5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(0)), static_cast<System::Int32>(static_cast<System::Byte>(120)),
                   static_cast<System::Int32>(static_cast<System::Byte>(215)));
               this->button5->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
               this->button5->ForeColor = System::Drawing::Color::White;
               this->button5->Location = System::Drawing::Point(210, 47);
               this->button5->Name = L"button5";
               this->button5->Size = System::Drawing::Size(130, 24);
               this->button5->TabIndex = 4;
               this->button5->Text = L"Apply";
               this->button5->UseVisualStyleBackColor = false;
               this->button5->Click += gcnew System::EventHandler(this, &trafficSimulation::button5_Click);
               // 
               // vScrollBar1
               // 
               this->vScrollBar1->Location = System::Drawing::Point(845, 0);
               this->vScrollBar1->Name = L"vScrollBar1";
               this->vScrollBar1->Size = System::Drawing::Size(20, 605);
               this->vScrollBar1->TabIndex = 100;
               this->vScrollBar1->Scroll += gcnew System::Windows::Forms::ScrollEventHandler(this, &trafficSimulation::vScrollBar1_Scroll);
               // 
               // hScrollBar1
               // 
               this->hScrollBar1->Location = System::Drawing::Point(0, 605);
               this->hScrollBar1->Name = L"hScrollBar1";
               this->hScrollBar1->Size = System::Drawing::Size(845, 20);
               this->hScrollBar1->TabIndex = 101;
               this->hScrollBar1->Scroll += gcnew System::Windows::Forms::ScrollEventHandler(this, &trafficSimulation::hScrollBar1_Scroll);
               // 
               // trafficSimulation
               // 
               this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
               this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
               this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(30)), static_cast<System::Int32>(static_cast<System::Byte>(30)),
                   static_cast<System::Int32>(static_cast<System::Byte>(30)));
               this->ClientSize = System::Drawing::Size(1236, 630);
               this->Controls->Add(this->panel1);
               this->Controls->Add(this->panel2);
               this->Controls->Add(this->vScrollBar1);
               this->Controls->Add(this->hScrollBar1);
               this->Name = L"trafficSimulation";
               this->Text = L"Traffic Flow Optimization System";
               this->Load += gcnew System::EventHandler(this, &trafficSimulation::trafficSimulation_Load);
               this->panel1->ResumeLayout(false);
               this->panel2->ResumeLayout(false);
               this->panel2->PerformLayout();
               this->groupBox4->ResumeLayout(false);
               this->groupBox4->PerformLayout();
               this->groupBox1->ResumeLayout(false);
               this->groupBox1->PerformLayout();
               this->groupBox6->ResumeLayout(false);
               this->groupBox6->PerformLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar1))->EndInit();
               this->groupBox2->ResumeLayout(false);
               this->groupBox2->PerformLayout();
               this->groupBox3->ResumeLayout(false);
               this->groupBox3->PerformLayout();
               this->ResumeLayout(false);

           }
#pragma endregion
    };
}
//GUIObserver::update implementation
inline void GUIObserver::update()
{
    SemesterProject::trafficSimulation^ f =
        safe_cast<SemesterProject::trafficSimulation^>((System::Object^)form);
    f->onBackendUpdate();
}
