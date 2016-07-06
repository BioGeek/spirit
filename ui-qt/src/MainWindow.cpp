#include <QtWidgets>

#include "MainWindow.h"
#include "PlotWidget.h"

#include "Vectormath.h"
#include "Configurations.h"
#include "Optimizer.h"
#include "IO.h"

#include "Solver_LLG.h"
#include "Solver_GNEB.h"
#include "Logging.h"
#include "Timing.h"
#include "Threading.h"

#include <thread>

MainWindow::MainWindow(std::shared_ptr<Data::Spin_System_Chain> c)
{
	this->c = c;
	this->s = this->c->images[this->c->active_image];
	this->spinWidget = new Spin_Widget(this->c);
	//this->spinWidgetGL = new Spin_Widget_GL(s);
	this->settingsWidget = new SettingsWidget(this->c);
	this->plotsWidget = new PlotsWidget(this->c);
	this->debugWidget = new DebugWidget(this->c);

	//this->setFocus(Qt::StrongFocus);
	this->setFocusPolicy(Qt::StrongFocus);

    #ifdef Q_OS_MAC
        this->setStyleSheet("QWidget{font-size:10pt}");
    #else
        this->setStyleSheet("QWidget{font-size:8pt}");
    #endif
    
	// Setup User Interface
    this->setupUi(this);

	// Tabify DockWidgets for Plots and Debug
	this->tabifyDockWidget(this->dockWidget_Plots, this->dockWidget_Debug);
	this->dockWidget_Plots->raise();
	this->dockWidget_Debug->hide();

	// Read Window settings of last session
	readSettings();

	// Add Widgets to UIs grids
	this->gridLayout->addWidget(this->spinWidget, 0, 0, 1, 1);
	this->dockWidget_Settings->setWidget(this->settingsWidget);
	this->dockWidget_Plots->setWidget(this->plotsWidget);
	this->dockWidget_Debug->setWidget(this->debugWidget);

	// Read Iterate State form Spin System
	if (s->iteration_allowed == false)
	{
		this->pushButton_PlayPause->setText("Play");
	}
	else
	{
		this->pushButton_PlayPause->setText("Pause");
		//std::thread(Engine::SIB::Iterate, s, 2000000, 5000).detach();
	}
	
	/*
    // Create Widgets
    createWidgets(s);
    
    // Create Stuff
    createActions();
    //createMenus();    // these fail for some reason... maybe add resource stuff later on
    //createToolBars(); // these fail for some reason... maybe add resource stuff later on
    createStatusBar();

    connect(textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));

    setCurrentFile("");
    setUnifiedTitleAndToolBarOnMac(true);//*/
    

	// Buttons
	connect(this->lineEdit_Save_E, SIGNAL(returnPressed()), this, SLOT(save_EPressed()));
	connect(this->pushButton_Save_E, SIGNAL(clicked()), this, SLOT(save_EPressed()));
	connect(this->pushButton_StopAll, SIGNAL(clicked()), this, SLOT(stopallPressed()));
	connect(this->pushButton_PlayPause, SIGNAL(clicked()), this, SLOT(playpausePressed()));
	connect(this->pushButton_PreviousImage, SIGNAL(clicked()), this, SLOT(previousImagePressed()));
	connect(this->pushButton_NextImage, SIGNAL(clicked()), this, SLOT(nextImagePressed()));
    connect(this->pushButton_Reset, SIGNAL(clicked()), this, SLOT(resetPressed()));
    connect(this->pushButton_X, SIGNAL(clicked()), this, SLOT(xPressed()));
    connect(this->pushButton_Y, SIGNAL(clicked()), this, SLOT(yPressed()));
    connect(this->pushButton_Z, SIGNAL(clicked()), this, SLOT(zPressed()));


	// Image number
	// We use a regular expression (regex) to filter the input into the lineEdits
	QRegularExpression re("[\\d]*");
	QRegularExpressionValidator *number_vali = new QRegularExpressionValidator(re);
	this->lineEdit_ImageNumber->setValidator(number_vali);
	this->lineEdit_ImageNumber->setText(QString::number(1));

	// File Menu
	connect(this->actionLoad_Configuration, SIGNAL(triggered()), this, SLOT(load_Configuration()));
	connect(this->actionLoad_Spin_Configuration, SIGNAL(triggered()), this, SLOT(load_Spin_Configuration()));
	connect(this->actionLoad_SpinChain_Configuration, SIGNAL(triggered()), this, SLOT(load_SpinChain_Configuration()));
	connect(this->actionSave_Energies, SIGNAL(triggered()), this, SLOT(save_Energies()));
	connect(this->action_Save_Spin_Configuration, SIGNAL(triggered()), SLOT(save_Spin_Configuration()));
	connect(this->actionSave_SpinChain_Configuration, SIGNAL(triggered()), this, SLOT(save_SpinChain_Configuration()));

	// View Menu
	connect(this->actionShow_Settings, SIGNAL(triggered()), this, SLOT(view_toggleSettings()));
	connect(this->actionShow_Plots, SIGNAL(triggered()), this, SLOT(view_togglePlots()));
	connect(this->actionShow_Debug, SIGNAL(triggered()), this, SLOT(view_toggleDebug()));

	// Help Menu
	connect(this->actionKey_Bindings, SIGNAL(triggered()), this, SLOT(keyBindings()));	
	connect(this->actionAbout_this_Application, SIGNAL(triggered()), this, SLOT(about()));


    // Event Filter
    //this->installEventFilter(this);
	//this->statusBar->showMessage(tr("Ready"));
	//statusBar()->showMessage(tr("Ready"));


	// Set up Update Timer for Plots
	//m_timer_plots = new QTimer(this);
	//connect(m_timer_plots, &QTimer::timeout, this->plotsWidget->energyPlot, &PlotWidget::update);	// this currently resets the user's interaction (movement, zoom)
	//m_timer_plots->start(100);

	// Set up Update Timer for Spin Visualisation
	//m_timer_spins = new QTimer(this);
	////connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	//connect(m_timer_spins, &QTimer::timeout, this->spinWidget, &Spin_Widget::update);
	//m_timer_spins->start(100);

	// Set up Update Timer
	//m_timer_debug = new QTimer(this);
	//connect(m_timer_debug, &QTimer::timeout, this->debugWidget, &DebugWidget::update);
	//m_timer_debug->start(200);

}


//bool MainWindow::eventFilter(QObject *object, QEvent *event)
//{
//	if (object == lineEdit && event->type() == QEvent::FocusOut)
//	{
//		std::cout << "eventFilter" << std::endl;
//	}
//	return false; // Pass the event along (don't consume it)
//}

void MainWindow::keyPressEvent(QKeyEvent *k)
{
	// Key Sequences
	if (k->matches(QKeySequence::Copy))
	{
		// Copy a Spin System
		image_clipboard = std::shared_ptr<Data::Spin_System>(new Data::Spin_System(*s.get()));
		Utility::Log.Send(Utility::Log_Level::INFO, Utility::Log_Sender::GUI, "Copied image " + std::to_string(c->active_image) + " to clipboard");
	}
	else if (k->matches(QKeySequence::Cut))
	{
		if (c->noi > 1)
		{
			s->iteration_allowed = false;
			c->iteration_allowed = false;
			if (Utility::Threading::llg_threads[s].joinable()) {
				Utility::Threading::llg_threads[s].join();
			}
			if (Utility::Threading::gneb_threads[c].joinable()) {
				Utility::Threading::gneb_threads[c].join();
			}

			// Cut a Spin System
			image_clipboard = std::shared_ptr<Data::Spin_System>(new Data::Spin_System(*s.get()));

			int idx = c->active_image;
			if (idx > 0) this->previousImagePressed();
			//else this->nextImagePressed();

			this->c->Delete_Image(idx);

			Utility::Log.Send(Utility::Log_Level::INFO, Utility::Log_Sender::GUI, "Cut image " + std::to_string(c->active_image) + " to clipboard");
		}
	}
	else if (k->matches(QKeySequence::Paste))
	{
		// Paste a Spin System

		s->iteration_allowed = false;
		c->iteration_allowed = false;
		if (Utility::Threading::llg_threads[s].joinable()) {
			Utility::Threading::llg_threads[s].join();
		}
		if (Utility::Threading::gneb_threads[c].joinable()) {
			Utility::Threading::gneb_threads[c].join();
		}

		if (image_clipboard.get())
		{
			s = image_clipboard;
			c->Replace_Image(c->active_image, image_clipboard);
			Utility::Log.Send(Utility::Log_Level::INFO, Utility::Log_Sender::GUI, "Pasted image " + std::to_string(c->active_image) + " from clipboard");
		}
		else Utility::Log.Send(Utility::Log_Level::L_ERROR, Utility::Log_Sender::GUI, "Tried to paste image " + std::to_string(c->active_image) + " from clipboard but no image was found");
	}

	// Custom Key Sequences
	else if (k->modifiers() & Qt::ControlModifier)
	{
		std::shared_ptr<Data::Spin_System> newImage;
		switch (k->key())
		{
		case Qt::Key_Left:
			if (image_clipboard.get())
			{
				s = image_clipboard;
				this->c->Insert_Image_Before(this->c->active_image, image_clipboard);
				//this->previousImagePressed();
				Utility::Log.Send(Utility::Log_Level::INFO, Utility::Log_Sender::GUI, "Pasted image before " + std::to_string(c->active_image) + " from clipboard");
			}
			else Utility::Log.Send(Utility::Log_Level::L_ERROR, Utility::Log_Sender::GUI, "Tried to paste image before " + std::to_string(c->active_image) + " from clipboard but no image was found");
			break;

		case Qt::Key_Right:
			if (image_clipboard.get())
			{
				s = image_clipboard;
				this->c->Insert_Image_After(this->c->active_image, image_clipboard);
				this->nextImagePressed();
				Utility::Log.Send(Utility::Log_Level::INFO, Utility::Log_Sender::GUI, "Pasted image after " + std::to_string(c->active_image) + " from clipboard");
			}
			else Utility::Log.Send(Utility::Log_Level::L_ERROR, Utility::Log_Sender::GUI, "Tried to paste image after " + std::to_string(c->active_image) + " from clipboard but no image was found");
			break;
		}
	}
	
	// Single Keys
	else
	switch (k->key())
	{
		case Qt::Key_Escape:
			this->setFocus();
			break;

		case Qt::Key_Up:
			break;

		case Qt::Key_Left:
			this->previousImagePressed();
			break;

		case Qt::Key_Right:
			this->nextImagePressed();
			break;

		case Qt::Key_Down:
			break;

		case Qt::Key_0:
			break;

		case Qt::Key_Space:
			this->playpausePressed();
			break;

		case Qt::Key_F1:
			this->keyBindings();
			break;

		case Qt::Key_F2:
			this->view_toggleSettings();
			break;

		case Qt::Key_F3:
			this->view_togglePlots();
			break;

		case Qt::Key_F4:
			this->view_toggleDebug();
			break;

		case Qt::Key_1:
			this->settingsWidget->SelectTab(0);
			break;

		case Qt::Key_2:
			this->settingsWidget->SelectTab(1);
			break;

		case Qt::Key_3:
			this->settingsWidget->SelectTab(2);
			break;

		case Qt::Key_4:
			this->settingsWidget->SelectTab(3);
			break;

		case Qt::Key_5:
			this->settingsWidget->SelectTab(4);
			break;

		case Qt::Key_Delete:
			if (c->noi > 1)
			{
				s->iteration_allowed = false;
				c->iteration_allowed = false;
				if (Utility::Threading::llg_threads[s].joinable()) {
					Utility::Threading::llg_threads[s].join();
				}
				if (Utility::Threading::gneb_threads[c].joinable()) {
					Utility::Threading::gneb_threads[c].join();
				}

				int idx = c->active_image;
				if (idx > 0) this->previousImagePressed();
				//else this->nextImagePressed();
				this->c->Delete_Image(idx);

				Utility::Log.Send(Utility::Log_Level::INFO, Utility::Log_Sender::GUI, "Deleted image " + std::to_string(c->active_image));
			}
			break;
	}
}


void MainWindow::stopallPressed()
{
	this->return_focus();
	Utility::Log.Send(Utility::Log_Level::DEBUG, Utility::Log_Sender::GUI, std::string("Button: stopall"));
	
	c->iteration_allowed = false;

	for (int i = 0; i < c->noi; ++i)
	{
		c->images[i]->iteration_allowed = false;
	}

	this->pushButton_PlayPause->setText("Play");
}

void MainWindow::playpausePressed()
{
	this->return_focus();
	Utility::Log.Send(Utility::Log_Level::DEBUG, Utility::Log_Sender::GUI, std::string("Button: playpause"));

	this->c->Update_Data();

	std::shared_ptr<Engine::Optimizer> optim;
	if (this->comboBox_Optimizer->currentText() == "SIB")
	{
		optim = std::shared_ptr<Engine::Optimizer>(new Engine::Optimizer_SIB());
	}
	else if (this->comboBox_Optimizer->currentText() == "Heun")
	{
		optim = std::shared_ptr<Engine::Optimizer>(new Engine::Optimizer_Heun());
	}
	else if (this->comboBox_Optimizer->currentText() == "CG")
	{
		optim = std::shared_ptr<Engine::Optimizer>(new Engine::Optimizer_CG());
	}
	else if (this->comboBox_Optimizer->currentText() == "QM")
	{
		optim = std::shared_ptr<Engine::Optimizer>(new Engine::Optimizer_QM());
	}

	if (this->comboBox_Solver->currentText() == "LLG")
	{
		if (s->iteration_allowed)
		{
			this->pushButton_PlayPause->setText("Play");
			s->iteration_allowed = false;
			c->iteration_allowed = false;
		}
		else
		{
			this->pushButton_PlayPause->setText("Pause");
			if (Utility::Threading::llg_threads[s].joinable()) {
				s->iteration_allowed = false;
				Utility::Threading::llg_threads[s].join();
			}
			s->iteration_allowed = true;
			c->iteration_allowed = false;
            auto g = new Engine::Solver_LLG(this->c, optim);
			Utility::Threading::llg_threads[s] = std::thread(&Engine::Solver_LLG::Iterate, g, 2000000, 5000);
		}
	}
	else if (this->comboBox_Solver->currentText() == "GNEB")
	{
		if (c->iteration_allowed)
		{
			this->pushButton_PlayPause->setText("Play");
			for (int i = 0; i < c->noi; ++i)
			{
				c->images[i]->iteration_allowed = false;
			}
			c->iteration_allowed = false;
		}
		else
		{
			this->pushButton_PlayPause->setText("Pause");
			for (int i = 0; i < c->noi; ++i)
			{
				c->images[i]->iteration_allowed = false;
			}
			if (Utility::Threading::gneb_threads[c].joinable()) {
				c->iteration_allowed = false;
				Utility::Threading::gneb_threads[c].join();
			}
			c->iteration_allowed = true;
            auto g = new Engine::Solver_GNEB(this->c, optim);
			Utility::Threading::gneb_threads[c] = std::thread(&Engine::Solver_GNEB::Iterate, g, 2000000, 5000);
		}
	}
	else if (this->comboBox_Solver->currentText() == "MMF")
	{
		Utility::Log.Send(Utility::Log_Level::WARNING, Utility::Log_Sender::GUI, std::string("MMF selected, but not yet implemented! Not doing anything..."));
	}
}


void MainWindow::previousImagePressed()
{
	this->return_focus();
	if (this->c->active_image > 0)
	{
		// Change active image!
		c->active_image--;
		this->lineEdit_ImageNumber->setText(QString::number(c->active_image+1));
		this->s = c->images[c->active_image];
		// Update Play/Pause Button
		if (this->s->iteration_allowed || this->c->iteration_allowed) this->pushButton_PlayPause->setText("Pause");
		else this->pushButton_PlayPause->setText("Play");

		// Update Image-dependent Widgets
		//this->spinWidget->update();
		this->settingsWidget->update();
		this->plotsWidget->update();
		this->debugWidget->update();

		/*this->spinWidget = new Spin_Widget(s);
		this->settingsWidget = new SettingsWidget(s);
		this->plotsWidget = new PlotsWidget(c);
		this->debugWidget = new DebugWidget(s);

		this->gridLayout->addWidget(this->spinWidget, 0, 0, 1, 1);
		this->dockWidget_Settings->setWidget(this->settingsWidget);
		this->dockWidget_Plots->setWidget(this->plotsWidget);
		this->dockWidget_Debug->setWidget(this->debugWidget);*/
	}
}


void MainWindow::nextImagePressed()
{
	this->return_focus();
	if (this->c->active_image < this->c->noi-1)
	{
		// Change active image
		c->active_image++;
		this->lineEdit_ImageNumber->setText(QString::number(c->active_image+1));
		this->s = c->images[c->active_image];
		// Update Play/Pause Button
		if (this->s->iteration_allowed || this->c->iteration_allowed) this->pushButton_PlayPause->setText("Pause");
		else this->pushButton_PlayPause->setText("Play");

		// Update Image-dependent Widgets
		//this->spinWidget->update();
		this->settingsWidget->update();
		this->plotsWidget->update();
		this->debugWidget->update();

		/*this->spinWidget = new Spin_Widget(s);
		this->settingsWidget = new SettingsWidget(s);
		this->plotsWidget = new PlotsWidget(c);
		this->debugWidget = new DebugWidget(s);

		this->gridLayout->addWidget(this->spinWidget, 0, 0, 1, 1);
		this->dockWidget_Settings->setWidget(this->settingsWidget);
		this->dockWidget_Plots->setWidget(this->plotsWidget);
		this->dockWidget_Debug->setWidget(this->debugWidget);*/
	}
}


void MainWindow::resetPressed()
{
    /*this->spinWidget->SetCameraToDefault();
	this->spinWidget->update();*/
}

void MainWindow::xPressed()
{
    /*this->spinWidget->SetCameraToX();
	this->spinWidget->update();*/
}

void MainWindow::yPressed()
{
    /*this->spinWidget->SetCameraToY();
	this->spinWidget->update();*/
}

void MainWindow::zPressed()
{
    /*this->spinWidget->SetCameraToZ();
	this->spinWidget->update();*/
}

void MainWindow::view_toggleDebug()
{
	if (this->dockWidget_Debug->isVisible()) this->dockWidget_Debug->hide();
	else this->dockWidget_Debug->show();
}

void MainWindow::view_togglePlots()
{
	if (this->dockWidget_Plots->isVisible()) this->dockWidget_Plots->hide();
	else this->dockWidget_Plots->show();
}

void MainWindow::view_toggleSettings()
{
	if (this->dockWidget_Settings->isVisible()) this->dockWidget_Settings->hide();
	else this->dockWidget_Settings->show();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About JuSpin"),
		QString::fromLatin1("The <b>JuSpin</b> application incorporates intuitive visualisation,<br>"
			"powerful <b>Spin Dynamics</b> and <b>Nudged Elastic Band</b> tools<br>"
			"into a cross-platform user interface.<br>"
			"<br>"
			"Libraries used are<br>"
			"  - VTK 7<br>"
			"  - QT 5.5<br>"
			"<br>"
			"This has been developed by<br>"
			"  - Gideon M�ller (<a href=\"mailto:g.mueller@fz-juelich.de\">g.mueller@fz-juelich.de</a>)<br>"
			"  - Daniel Sch�rhoff (<a href=\"mailto:d.schuerhoff@fz-juelich.de\">d.schuerhoff@fz-juelich.de</a>)<br>"
			"at the Institute for Advanced Simulation 1 of the Forschungszentrum J�lich.<br>"
			"For more information about us, visit the <a href=\"http://www.fz-juelich.de/pgi/pgi-1/DE/Home/home_node.html\">IAS-1 Website</a><br>"
			"<br>"
			"<b>Copyright 2016</b><br>"));
}

void MainWindow::keyBindings()
{
	QMessageBox::about(this, tr("JuSpin UI Key Bindings"),
		QString::fromLatin1("The <b>Key Bindings</b> are as follows:<br>"
			"<br>"
			" - <b>F1</b>:      Show this<br>"
			" - <b>F2</b>:      Toggle Settings<br>"
			" - <b>F3</b>:      Toggle Plots<br>"
			" - <b>F4</b>:      Toggle Debug<br>"
			"<br>"
			" - <b>1-5</b>:     Select Tab in Settings<br>"
			"<br>"
			" - <b>Arrows</b>:  Switch between arrows and chains<br>"
			" - <b>WASD</b>:    Move the camera around (not yet functional)<br>"
			" - <b>Space</b>:   Play/Pause<br>"
			" - <b>Escape</b>:  Try to return focus to main UI (does not always work)<br>"
			"<br>"
			" - <b>Ctrl+X</b>:           Cut   Image<br>"
			" - <b>Ctrl+C</b>:           Copy  Image<br>"
			" - <b>Ctrl+V</b>:           Paste Image at current index<br>"
			" - <b>Ctrl+Left/Right</b>:  Insert left/right of current index<br>"
			" - <b>Del</b>:              Delete image<br>"));
}

void MainWindow::return_focus()
{
	auto childWidgets = this->findChildren<QWidget *>();
	for (int i = 0; i <childWidgets.count(); ++i)
	{
		childWidgets.at(i)->clearFocus();
	}
	/*this->pushButton_PreviousImage->clearFocus();
	this->pushButton_NextImage->clearFocus();
	this->pushButton_Reset->clearFocus();
	this->pushButton_X->clearFocus();
	this->pushButton_Y->clearFocus();
	this->pushButton_Z->clearFocus();*/
}


/*
	Converts a QString to an std::string.
	This function is needed sometimes due to weird behaviour of QString::toStdString().
*/
std::string string_q2std(QString qs)
{
	auto bytearray = qs.toLatin1();
	const char *c_fileName = bytearray.data();
	return std::string(c_fileName);
}

void MainWindow::save_Spin_Configuration()
{
	auto fileName = QFileDialog::getSaveFileName(this, tr("Save Spin Configuration"), "./output", tr("Spin Configuration (*.txt)"));
	if (!fileName.isEmpty()) {
		Utility::IO::Append_Spin_Configuration(this->s, 0, string_q2std(fileName));
	}
}
void MainWindow::load_Spin_Configuration()
{
	auto fileName = QFileDialog::getOpenFileName(this, tr("Load Spin Configuration"), "./input", tr("Spin Configuration (*.txt)"));
	if (!fileName.isEmpty()) {
		Utility::IO::Read_Spin_Configuration(this->s, string_q2std(fileName));
	}
}

void MainWindow::save_SpinChain_Configuration()
{
	auto fileName = QFileDialog::getSaveFileName(this, tr("Save SpinChain Configuration"), "./output", tr("Spin Configuration (*.txt)"));
	if (!fileName.isEmpty()) {
		Utility::IO::Save_SpinChain_Configuration(this->c, string_q2std(fileName));
	}
}

void MainWindow::load_SpinChain_Configuration()
{
	auto fileName = QFileDialog::getOpenFileName(this, tr("Load Spin Configuration"), "./input", tr("Spin Configuration (*.txt)"));
	if (!fileName.isEmpty()) {
		Utility::IO::Read_SpinChain_Configuration(this->c, string_q2std(fileName));
	}
}


void MainWindow::save_Energies()
{
	this->return_focus();
	auto fileName = QFileDialog::getSaveFileName(this, tr("Save Energies"), "./output", tr("Text (*.txt)"));
	if (!fileName.isEmpty()) {
		Utility::IO::Save_Energies(*c, 0, string_q2std(fileName));
	}
}

void MainWindow::save_EPressed()
{
	std::string fullName = "output/";
	std::string fullNameSpins = "output/";
	std::string fullNameInterpolated = "output/";

	// Get file info
	auto qFileName = lineEdit_Save_E->text();
	QFileInfo fileInfo(qFileName);
	
	// Construct the file names
	std::string fileName = string_q2std(fileInfo.baseName()) + "." + string_q2std(fileInfo.completeSuffix());
	std::string fileNameSpins = string_q2std(fileInfo.baseName()) + "_Spins." + string_q2std(fileInfo.completeSuffix());
	std::string fileNameInterpolated = string_q2std(fileInfo.baseName()) + "_Interpolated." + string_q2std(fileInfo.completeSuffix());

	// File names including path
	fullName.append(fileName);
	fullNameSpins.append(fileNameSpins);
	fullNameInterpolated.append(fileNameInterpolated);

	// Save Energies and Energies_Spins
	Utility::IO::Save_Energies(*c, 0, fullName);
	Utility::IO::Save_Energies_Spins(*c, fullNameSpins);
	Utility::IO::Save_Energies_Interpolated(*c, fullNameInterpolated);

	// Update File name in LineEdit if it fits the schema
	size_t found = fileName.find("Energies");
	if (found != std::string::npos) {
		int a = std::stoi(fileName.substr(found+9, 3)) + 1;
		char newName[20];
		snprintf(newName, 20, "Energies_%03i.txt", a);
		lineEdit_Save_E->setText(newName);
	}
}

void MainWindow::load_Configuration()
{
	int idx_img = c->active_image;
	// Read System
	auto fileName = QFileDialog::getOpenFileName(this, tr("Open Config"), "./input", tr("Config (*.cfg)"));
	if (!fileName.isEmpty()) {
		std::shared_ptr<Data::Spin_System> sys = Utility::IO::Spin_System_from_Config(string_q2std(fileName));
		// Filter for unacceptable differences to other systems in the chain
		bool acceptable = true;
		for (int i = 0; i < c->noi; ++i)
		{
			if (c->images[i]->nos != sys->nos) acceptable = false;
			// Currently the SettingsWidget does not support different images being isotropic AND anisotropic at the same time
			if (c->images[i]->is_isotropic != sys->is_isotropic) acceptable = false;
		}
		// Set current image
		if (acceptable)
		{
			this->c->images[idx_img] = sys;
			Utility::Configurations::Random(*sys);
		}
		else QMessageBox::about(this, tr("About JuSpin"),
			tr("The resulting Spin System would have different NOS\n"
				"or isotropy status than one or more of the other\n"
				"images in the chain!\n"
				"\n"
				"The system has thus not been reset!"));
	}
}

void MainWindow::readSettings()
{
	QSettings settings("QtProject", "Application Example");
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	resize(size);
	move(pos);
}

void MainWindow::writeSettings()
{
	QSettings settings("QtProject", "Application Example");
	settings.setValue("pos", pos());
	settings.setValue("size", size());
}


/*bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{ 
    if (event->type() == QEvent::KeyPress)
    {
        if(obj == spins)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            spins->keyPressEvent(keyEvent);
        }
    }
    return QObject::eventFilter(obj, event);
}*/


void MainWindow::closeEvent(QCloseEvent *event)
{
	c->iteration_allowed = false;
	if (Utility::Threading::gneb_threads[c].joinable()) Utility::Threading::gneb_threads[c].join();
	for (int isystem = 0; isystem < (int)c->images.size(); ++isystem)
	{
		c->images[isystem]->iteration_allowed = false;
		if (Utility::Threading::llg_threads[c->images[isystem]].joinable())
		{
			Utility::Threading::llg_threads[c->images[isystem]].join();
		}	
	}
	
    //if (maybeSave()) {
        writeSettings();
        event->accept();
    /*} else {
        event->ignore();
    }*/
}
/*
void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();
    else
        return false;

    return saveFile(files.at(0));
}



void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}


void MainWindow::createWidgets(Spin_System * s)
{
    // Create Widgets
    textEdit = new QPlainTextEdit;
    //spins = new SpinWidget(s);
    spinWidget = new Spin_Widget(s);
    disableGLHiDPI(spinWidget->winId()); // thanks to http://public.kitware.com/pipermail/vtkusers/2015-February/090117.html Retina displays on OS X have a bug due to some QT internals
    
    QPushButton *renderButton = new QPushButton(tr("Render"));
    QPushButton *renderButton2 = new QPushButton(tr("Render2"));
    
    
    
    
    // Set central widget with layout
    QWidget *centralWidget = new QWidget();
    QGridLayout *layout = new QGridLayout();
    centralWidget->setLayout (layout);
    setCentralWidget(centralWidget);
    //setCentralWidget(spins);
    //setCentralWidget(example);
    

    // Set layout
    //layout->addWidget (spins, 0, 0, 1, 1);
    layout->addWidget (spinWidget, 0, 0, 1, 1);
    layout->addWidget (textEdit, 0, 1, 1, 1);
    layout->addWidget (renderButton2, 1,1, 1, 1);
    layout->addWidget (renderButton, 1, 0, 1, 1);
}


void MainWindow::createActions()
{
    // Key Presses
    //connect(this, SIGNAL(keyPressEvent(QKeyEvent*)), spins, SLOT(keyPressEvent(QKeyEvent*)));
    
    
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
            
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings("QtProject", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QSettings settings("QtProject", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

bool MainWindow::maybeSave()
{
    if (textEdit->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}*/