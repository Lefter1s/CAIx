/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011-2012
 * The Bitcoin Developers 2011-2012
 */
#include "bitcoingui.h"
#include "transactiontablemodel.h"
#include "addressbookpage.h"
#include "sendcoinsdialog.h"
#include "signverifymessagedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "tutoStackDialog.h"
#include "tutoWriteDialog.h"
#include "transactiondescdialog.h"
#include "addresstablemodel.h"
#include "transactionview.h"
#include "overviewpage.h"
#include "statisticspage.h"
#include "blockbrowser.h"
#include "poolbrowser.h"
#include "bitcoinunits.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "notificator.h"
#include "guiutil.h"
#include "rpcconsole.h"
#include "wallet.h"

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QStackedWidget>
#include <QDateTime>
#include <QMovie>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>
#include <QDragEnterEvent>
#include <QUrl>
#include <QStyle>
#include <QFont>
#include <QFontDatabase>

#include <iostream>

extern CWallet* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
extern unsigned int nTargetSpacing;
double GetPoSKernelPS();
int convertmode = 0;

BitcoinGUI::BitcoinGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    changePassphraseAction(0),
    unlockWalletAction(0),
    lockWalletAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0)
{
    setFixedSize(1000, 600);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowTitle(tr("CAIx") + " " + tr("Wallet"));
    setObjectName("CAIx-qt");
    setStyleSheet("#CAIx-qt {background-color:#fbf9f6; font-family:'Open Sans'; border-style: outset; border-color: #33363B; border-width: 1px;}");

#ifndef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":icons/bitcoin"));
    setWindowIcon(QIcon(":icons/bitcoin"));
#else
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    // Include Fonts
    includeFonts();

    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();

    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

    // Create additional content for pages
    createContent();

    // Create navigation tabs
    overviewPage = new OverviewPage();
    statisticsPage = new StatisticsPage(this);
	blockBrowser = new BlockBrowser(this);
	poolBrowser = new PoolBrowser(this);
    transactionsPage = new QWidget(this);
    QVBoxLayout *transactionVbox = new QVBoxLayout();
    transactionView = new TransactionView(this);
    transactionVbox->addWidget(transactionView);
    transactionVbox->setContentsMargins(20,0,20,20);
    transactionsPage->setLayout(transactionVbox);
    transactionsPage->setStyleSheet("background:rgb(255,249,247)");
    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab);
    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab);
    sendCoinsPage = new SendCoinsDialog(this);
    signVerifyMessageDialog = new SignVerifyMessageDialog(this);

    //Define Central Widget
    centralWidget = new QStackedWidget(this);
    centralWidget->setStyleSheet("margin-left:1px");
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(statisticsPage);
    centralWidget->addWidget(blockBrowser);
	centralWidget->addWidget(poolBrowser);
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
    centralWidget->addWidget(receiveCoinsPage);
    centralWidget->addWidget(sendCoinsPage);
    centralWidget->addWidget(settingsPage);
    centralWidget->setMaximumWidth(998);
    centralWidget->setMaximumHeight(500);
    setCentralWidget(centralWidget);

    // Create status bar notification icons
    labelEncryptionIcon = new QLabel();
    labelStakingIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    actionConvertIcon = new QAction(QIcon(":/icons/changevalcaix"), tr(""), this);
    actionConvertIcon->setToolTip("Convert currency");

    // Get current staking status
    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingIcon()));
        timerStakingIcon->start(30 * 1000);
        updateStakingIcon();
    }

    // Progress bar and label for blocks download, disabled for current CAIx wallet
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new QProgressBar();

    // Create toolbars
    createToolBars();

    // When clicking the current currency logo, the currency will be converted into CAIx, BTC or USD
    connect(actionConvertIcon, SIGNAL(triggered()), this, SLOT(sConvert()));

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    // RPC Console
    rpcConsole = new RPCConsole(this);
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));
    connect(openRPCConsoleAction2, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    // Clicking on "Verify Message" in the address book sends you to the verify message tab
    connect(addressBookPage, SIGNAL(verifyMessage(QString)), this, SLOT(gotoVerifyMessageTab(QString)));

    // Clicking on "Sign Message" in the receive coins page sends you to the sign message tab
    connect(receiveCoinsPage, SIGNAL(signMessage(QString)), this, SLOT(gotoSignMessageTab(QString)));

    //Go to overview page
    gotoOverviewPage();
}

BitcoinGUI::~BitcoinGUI()
{
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
}

void BitcoinGUI::includeFonts()
{
    QStringList list;
    list << "OpenSans-Regular.ttf" << "OpenSans-Bold.ttf" << "OpenSans-ExtraBold.ttf";
    int fontID(-1);
    bool fontWarningShown(false);
    for (QStringList::const_iterator constIterator = list.constBegin(); constIterator != list.constEnd(); ++constIterator) {
        QFile res(":/fonts/" + *constIterator);
        if (res.open(QIODevice::ReadOnly) == false) {
            if (fontWarningShown == false) {
                QMessageBox::warning(0, "Application", (QString)"Impossible to open " + QChar(0x00AB) + *constIterator + QChar(0x00BB) + ".");
                fontWarningShown = true;
            }
        } else {
            fontID = QFontDatabase::addApplicationFontFromData(res.readAll());
            if (fontID == -1 && fontWarningShown == false) {
                QMessageBox::warning(0, "Application", (QString)"Impossible to open " + QChar(0x00AB) + *constIterator + QChar(0x00BB) + ".");
                fontWarningShown = true;
            }
        }
    }
}

void BitcoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    // Navigation bar actions
    overviewAction = new QAction(tr("&Dashboard"), this);
    overviewAction->setToolTip(tr("Show general overview"));
    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(tr("&Send"), this);
    sendCoinsAction->setToolTip(tr("Send coins to a CAIx address"));
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    sendCoinsAction->setObjectName("send");
    tabGroup->addAction(sendCoinsAction);

    receiveCoinsAction = new QAction(tr("&Receive"), this);
    receiveCoinsAction->setToolTip(tr("Receive addresses list"));
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    receiveCoinsAction->setCheckable(true);
    tabGroup->addAction(receiveCoinsAction);

    historyAction = new QAction(tr("&Transactions"), this);
    historyAction->setToolTip(tr("Browse transaction history"));
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

    addressBookAction = new QAction(tr("&Address Book"), this);
    addressBookAction->setToolTip(tr("Edit the list of stored addresses and labels"));
    addressBookAction->setCheckable(true);
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(addressBookAction);

    poolAction = new QAction(tr("&Market Data"), this);
    poolAction->setToolTip(tr("Show market data"));
    poolAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
    poolAction->setCheckable(true);
    tabGroup->addAction(poolAction);

    blockAction = new QAction(tr("&Block Explorer"), this);
    blockAction->setToolTip(tr("Explore the CAIx blockchain"));
    blockAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_8));
    blockAction->setCheckable(true);
    tabGroup->addAction(blockAction);

    actionmenuAction = new QAction(tr("&Actions"), this);
    actionmenuAction->setToolTip(tr("Multiple wallet actions"));
    actionmenuAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_9));
    actionmenuAction->setCheckable(true);
    tabGroup->addAction(actionmenuAction);

    settingsAction = new QAction(tr("&Actions"), this);
    settingsAction->setToolTip(tr("Multiple wallet actions"));
    settingsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_9));
    settingsAction->setCheckable(true);
    tabGroup->addAction(settingsAction);

    statisticsAction = new QAction(tr("&Statistics"), this);
    statisticsAction->setToolTip(tr("View CAIx statistics"));
    statisticsAction->setCheckable(true);
    tabGroup->addAction(statisticsAction);
    
    optionsAction = new QAction(tr("&Settings"), this);
    optionsAction->setToolTip(tr("Modify settings for CAIx wallet"));
    tabGroup->addAction(optionsAction);

    // Connect actions to functions
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(blockAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(blockAction, SIGNAL(triggered()), this, SLOT(gotoBlockBrowser()));
    connect(poolAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
	connect(poolAction, SIGNAL(triggered()), this, SLOT(gotoPoolBrowser()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(statisticsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(statisticsAction, SIGNAL(triggered()), this, SLOT(gotoStatisticsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(gotoSettingsPage()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));

    // Upper right toolbar actions
    toggleHideAction = new QAction(QIcon(":/icons/mini"), tr("&Minimalize"), this);

    quitAction = new QAction(QIcon(":/icons/quit"), tr("&Exit"), this);
    quitAction->setToolTip(tr("Close"));

    quitAction2 = new QAction(QIcon(":/icons/quit"), tr("&Exit"), this);
    quitAction2->setToolTip(tr("Quit application"));
    quitAction2->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    // ACTIONS tab actions
    tutoStackAction = new QAction(QIcon(":/images/howtostake"),tr("&Staking tutorial"), this);
    aboutAction = new QAction(QIcon(":/images/aboutcaix"), tr("&About CAIx"), this);
    aboutAction->setToolTip(tr("Show information about CAIx"));
    aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("&About Qt"), this);
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    encryptWalletAction = new QAction(QIcon(":/images/encryptwallet"), tr("&Encrypt wallet"), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/images/backupwallet"), tr("&Backup wallet"), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/images/newpassphrase"), tr("&Change passphrase"), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(QIcon(":/images/unlockwallet"), tr("&Unlock wallet"), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet"));
    lockWalletAction = new QAction(QIcon(":/images/lockwallet"), tr("&Lock wallet"), this);
    lockWalletAction->setToolTip(tr("Lock wallet"));
    signMessageAction = new QAction(QIcon(":/images/signmessage"), tr("&Sign message"), this);
    verifyMessageAction = new QAction(QIcon(":/images/verifymessage"), tr("&Verify message"), this);
    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    exportAction->setToolTip(tr("Export the data in the current tab to a file"));
    openRPCConsoleAction = new QAction(QIcon(":/images/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));
    signMessageAction2 = new QAction(QIcon(":/icons/edit"), tr("&Sign message..."), this);
    verifyMessageAction2 = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);
    openRPCConsoleAction2 = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction2->setToolTip(tr("Open debugging and diagnostic console"));

#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("&File"));
    file->addAction(backupWalletAction);
    file->addAction(exportAction);
    file->addAction(signMessageAction);
    file->addAction(verifyMessageAction);
    file->addSeparator();
    file->addAction(quitAction2);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    settings->addAction(encryptWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addAction(unlockWalletAction);
    settings->addAction(lockWalletAction);
    settings->addSeparator();
    settings->addAction(optionsAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
#endif

    // Connect actions to slots
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(quitAction2, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(tutoStackAction, SIGNAL(triggered()), this, SLOT(tutoStackClicked()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction, SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(backupWalletAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
    connect(unlockWalletAction, SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(lockWalletAction, SIGNAL(triggered()), this, SLOT(lockWallet()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
    connect(signMessageAction2, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction2, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
}

void BitcoinGUI::createContent()
{
    //Create Actions Page Content
    settingsPage = new QWidget(this);
    settingsPage->setFixedSize(1000,600);
    settingsPage->setStyleSheet("QToolBar{border:0px;} QToolBar QToolButton{border:0px;}");

    QFrame *WalletOptions = new QFrame(settingsPage);
    WalletOptions->setGeometry(20,0,700,400);
    WalletOptions->setFixedSize(700,400);
    WalletOptions->setStyleSheet("background-image:url(:/images/walletoptions);background-repeat:no-repeat;");
    QFrame *WalletOptionsButtonFrame = new QFrame(WalletOptions);
    WalletOptionsButtonFrame->setGeometry(0,45,700,200);
    WalletOptionsButtonFrame->setStyleSheet("background-image:url(:/icons/transpix);background-repeat:no-repeat;");
    QToolBar *WalletOptionsToolbar = new QToolBar(WalletOptionsButtonFrame);
    WalletOptionsToolbar->setOrientation(Qt::Horizontal);
    WalletOptionsToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    WalletOptionsToolbar->setMovable(false);
    WalletOptionsToolbar->setIconSize(QSize(158,49));
    WalletOptionsToolbar->addAction(lockWalletAction);
    WalletOptionsToolbar->addAction(unlockWalletAction);
    WalletOptionsToolbar->addAction(encryptWalletAction);
    WalletOptionsToolbar->addAction(backupWalletAction);
    WalletOptionsToolbar->addAction(changePassphraseAction);

    QFrame *SystemFunctions = new QFrame(settingsPage);
    SystemFunctions->setGeometry(713,0,267,500);
    SystemFunctions->setFixedSize(270,500);
    SystemFunctions->setStyleSheet("background-image:url(:/images/systemfunctions);background-repeat:no-repeat;");
    QFrame *SystemFunctionsButtonFrame = new QFrame(SystemFunctions);
    SystemFunctionsButtonFrame->setGeometry(0,45,270,500);
    SystemFunctionsButtonFrame->setStyleSheet("background-image:url(:/icons/transpix);background-repeat:no-repeat;");
    QToolBar *SystemFunctionsToolbar = new QToolBar(SystemFunctionsButtonFrame);
    SystemFunctionsToolbar->setGeometry(53,0,270,450);
    SystemFunctionsToolbar->setOrientation(Qt::Vertical);
    SystemFunctionsToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    SystemFunctionsToolbar->setMovable(false);
    SystemFunctionsToolbar->setIconSize(QSize(158,49));
    SystemFunctionsToolbar->addAction(signMessageAction);
    SystemFunctionsToolbar->addAction(verifyMessageAction);
    SystemFunctionsToolbar->addAction(openRPCConsoleAction);

    QFrame *QuickSupport = new QFrame(settingsPage);
    QuickSupport->setGeometry(20,200,700,300);
    QuickSupport->setFixedSize(700,300);
    QuickSupport->setStyleSheet("background-image:url(:/images/quicksupport);background-repeat:no-repeat;");
    QFrame *QuickSupportButtonFrame = new QFrame(QuickSupport);
    QuickSupportButtonFrame->setGeometry(0,45,700,300);
    QuickSupportButtonFrame->setStyleSheet("background-image:url(:/icons/transpix);background-repeat:no-repeat;");
    QToolBar *QuickSupportToolbar = new QToolBar(QuickSupportButtonFrame);
    QuickSupportToolbar->setOrientation(Qt::Horizontal);
    QuickSupportToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    QuickSupportToolbar->setMovable(false);
    QuickSupportToolbar->setIconSize(QSize(158,49));
    QuickSupportToolbar->addAction(tutoStackAction);
    QuickSupportToolbar->addAction(aboutAction);  
}

void BitcoinGUI::createToolBars()
{
    // Create status bar
    addToolBarBreak(Qt::TopToolBarArea);
    QToolBar *statusBar = addToolBar(tr("Status bar"));
    addToolBar(Qt::TopToolBarArea,statusBar);
    statusBar->setOrientation(Qt::Horizontal);
    statusBar->setMovable(false);
    statusBar->setObjectName("statusBar");
    statusBar->setFixedSize(1000,28);
    statusBar->setIconSize(QSize(42,28));
    QWidget *addMargin = new QWidget(this);
    addMargin->setFixedWidth(35);
    statusBar->addWidget(addMargin);
    statusBar->addWidget(labelEncryptionIcon);
    statusBar->addWidget(labelConnectionsIcon);
    statusBar->addWidget(labelBlocksIcon);
    statusBar->addWidget(labelStakingIcon);
    statusBar->setStyleSheet("QToolBar QToolButton {border:0px;} QToolBar{background-image: url(:/images/header-top); background-repeat:no-repeat; border-style: none outset none outset; border-width:1px; border-color:#33363B;}");
    //insertToolBarBreak(statusBar);

    //Create toolbar to convert, minimalise and quit
    QToolBar *quitBar = addToolBar(tr("Minimalise and Quit bar"));
    quitBar->setObjectName("quitBar");
    addToolBar(Qt::RightToolBarArea,quitBar);
    quitBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    quitBar->setOrientation(Qt::Horizontal);
    quitBar->setMovable(false);
    quitBar->setIconSize(QSize(35,28));
    quitBar->setFixedSize(128,28);
    QWidget* quitSpacer = new QWidget();
    quitSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    quitBar->addWidget(quitSpacer);
    quitBar->addAction(actionConvertIcon);
    quitBar->addAction(toggleHideAction);
    quitBar->addAction(quitAction);
    quitBar->setStyleSheet("QToolBar QToolButton {border:0px;} QToolBar { border-style: none outset none none; border-color: #33363B; border-width: 1px;}");
    QHBoxLayout *quitBarVbox = new QHBoxLayout();
    quitBarVbox->addWidget(quitBar);
    quitBarVbox->setContentsMargins(0,0,0,0);
    wId = new QWidget(this);
    wId->setFixedSize(128,28);
    wId->move(870,0);
    wId->setLayout(quitBarVbox);
    wId->setFocus();
    insertToolBarBreak(quitBar);

    QToolBar *navigationBar = addToolBar(tr("Navigation Bar"));
    navigationBar->setObjectName("navigationBar");
    addToolBar(Qt::TopToolBarArea,navigationBar);
    navigationBar->setOrientation(Qt::Horizontal);
    navigationBar->setMovable( false );
    navigationBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    navigationBar->setIconSize(QSize(30,54));
    navigationBar->setFixedSize(1000,60);
    navigationBar->addAction(overviewAction);
    navigationBar->addAction(sendCoinsAction);
    navigationBar->addAction(receiveCoinsAction);
    navigationBar->addAction(historyAction);
    navigationBar->addAction(addressBookAction);
    navigationBar->addAction(poolAction);
    navigationBar->addAction(blockAction);
    navigationBar->addAction(settingsAction);
    navigationBar->setStyleSheet("QToolBar {border-style: none outset none outset; border-width:1px; border-color:#33363B; background-image: url(:/images/menu); background-repeat:no-repeat;} QToolBar QToolButton:hover{color: #F37255;} QToolBar QToolButton:checked {color: #F37255;} QToolBar QToolButton{font-weight:bold; margin-bottom:13px; padding-left:8px; font-size:11px; font-family:'Open Sans Extrabold'; color:#848890; text-align:left; background:transparent; text-transform:uppercase; height:100%;}");
    insertToolBarBreak(navigationBar);

    //Export and Settings bar
    QToolBar *extraFunctionsBar = addToolBar(tr("Extra Functions bar"));
    extraFunctionsBar->setObjectName("extraFunctionsBar");
    addToolBar(Qt::RightToolBarArea,extraFunctionsBar);
    extraFunctionsBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    extraFunctionsBar->setOrientation(Qt::Horizontal);
    extraFunctionsBar->setMovable(false);
    extraFunctionsBar->setFixedWidth(200);
    extraFunctionsBar->setFixedHeight(60);
    extraFunctionsBar->setStyleSheet("QToolBar {border:0px;} QToolBar QToolButton:hover{color: #F37255;} QToolBar QToolButton:checked {color: #F37255;} QToolBar QToolButton{margin-bottom:13px; font-size:11px; font-family:'Open Sans Extrabold'; color:#33363B; text-align:right; background:transparent; text-transform:uppercase; height:100%; padding-left:20px;}");
    QWidget* extraSpacer = new QWidget();
    extraSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    extraFunctionsBar->addWidget(extraSpacer);
    extraFunctionsBar->addAction(exportAction);
    extraFunctionsBar->addAction(optionsAction);
    insertToolBarBreak(extraFunctionsBar);
    QHBoxLayout *extraFunctionsBarVbox = new QHBoxLayout();
    extraFunctionsBarVbox->addWidget(extraFunctionsBar);
    extraFunctionsBarVbox->setContentsMargins(0,0,0,0);
    wId2 = new QWidget(this);
    wId2->setFixedSize(200,60);
    wId2->move(798,28);
    wId2->setLayout(extraFunctionsBarVbox);
    wId2->setFocus();
}

void BitcoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Replace some strings and icons, when using the testnet
        if(clientModel->isTestNet())
        {
            setWindowTitle(windowTitle() + QString(" ") + tr("[testnet]"));
#ifndef Q_OS_MAC
            qApp->setWindowIcon(QIcon(":icons/bitcoin_testnet"));
            setWindowIcon(QIcon(":icons/bitcoin_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/bitcoin_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(tr("CAIx client") + QString(" ") + tr("[testnet]"));
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
                toggleHideAction->setIcon(QIcon(":/icons/toolbar_testnet"));
            }

            aboutAction->setIcon(QIcon(":/icons/toolbar_testnet"));
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks(), clientModel->getNumBlocksOfPeers());
        connect(clientModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        rpcConsole->setClientModel(clientModel);
        addressBookPage->setOptionsModel(clientModel->getOptionsModel());
        receiveCoinsPage->setOptionsModel(clientModel->getOptionsModel());
    }
}

void BitcoinGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);
        overviewPage->setModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        signVerifyMessageDialog->setModel(walletModel);
        statisticsPage->setModel(clientModel);
        blockBrowser->setModel(clientModel);
        poolBrowser->setModel(clientModel);
        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon pop-up for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));
    }
}

void BitcoinGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("CAIx client"));
    trayIcon->setIcon(QIcon(":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction2);
    trayIconMenu->addAction(verifyMessageAction2);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openRPCConsoleAction2);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction2);
#endif

    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHideAction->trigger();
    }
}
#endif

void BitcoinGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())\
    {
        return;
    }
    OptionsDialog dlg;
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void BitcoinGUI::sConvert()
{
    if (convertmode == 0)
    {
        actionConvertIcon->setIcon(QIcon(":/icons/changevaldollar").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        convertmode = 1;
    }
    else if (convertmode == 1)
    {
        actionConvertIcon->setIcon(QIcon(":/icons/changevalbtc").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        convertmode = 2;
    }
    else if (convertmode == 2)
    {
        actionConvertIcon->setIcon(QIcon(":/icons/changevalcaix").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        convertmode = 0;
    }
}

void BitcoinGUI::tutoWriteClicked()
{
    tutoWriteDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void BitcoinGUI::tutoStackClicked()
{
    tutoStackDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void BitcoinGUI::aboutClicked()
{
    AboutDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void BitcoinGUI::setNumConnections(int count)
{
    overviewPage->setNumberConnections(clientModel->getNumConnections());
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to CAIx network", "", count));
}

void BitcoinGUI::setNumBlocks(int count, int nTotalBlocks)
{
    QString strStatusBarWarnings = clientModel->getStatusBarWarnings();
    QString tooltip;

    labelBlocksIcon->setPixmap(QIcon(":/icons/not_synced").pixmap(STATUSBAR_wICONSIZE, STATUSBAR_hICONSIZE));
    labelBlocksIcon->setToolTip("Catchin up...");

    // don't show / hide progress bar and its label if we have no connection to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    if(count < nTotalBlocks)
    {
        //int nRemainingBlocks = nTotalBlocks - count;
        float nPercentageDone = count / (nTotalBlocks * 0.01f);

        if (strStatusBarWarnings.isEmpty())
        {
            progressBarLabel->setVisible(false);
            progressBar->setFormat(tr("%n%", "", nPercentageDone));
            progressBar->setMaximum(nTotalBlocks);
            progressBar->setValue(count);
            progressBar->setVisible(false);
        }

        tooltip = tr("Downloaded %1 of %2 blocks of transaction history (%3% done).").arg(count).arg(nTotalBlocks).arg(nPercentageDone, 0, 'f', 2);
    }
    else
    {
        if (strStatusBarWarnings.isEmpty())
            progressBarLabel->setVisible(false);

        progressBar->setVisible(false);
        tooltip = tr("Downloaded %1 blocks of transaction history.").arg(count);
    }

    // Override progressBarLabel text and hide progress bar, when we have warnings to display
    if (!strStatusBarWarnings.isEmpty())
    {
        progressBarLabel->setText(strStatusBarWarnings);
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);
    }

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(QDateTime::currentDateTime());
    QString text;

    // Represent time from last generated block in human readable text
    if(secs <= 0)
    {
        // Fully up to date. Leave text empty.
    }
    else if(secs < 60)
    {
        text = tr("%n second(s) ago","",secs);
    }
    else if(secs < 60*60)
    {
        text = tr("%n minute(s) ago","",secs/60);
    }
    else if(secs < 24*60*60)
    {
        text = tr("%n hour(s) ago","",secs/(60*60));
    }
    else
    {
        text = tr("%n day(s) ago","",secs/(60*60*24));
    }

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60 && count >= nTotalBlocks)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_wICONSIZE, STATUSBAR_hICONSIZE));
        overviewPage->showOutOfSyncWarning(false);
    }
    else
    {
        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/not_synced").pixmap(STATUSBAR_wICONSIZE, STATUSBAR_hICONSIZE));
        overviewPage->showOutOfSyncWarning(true);
    }

    if(!text.isEmpty())
    {
        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1.").arg(text);
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void BitcoinGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}

void BitcoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void BitcoinGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_OS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            QApplication::quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void BitcoinGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
    QString strMessage =
        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, nFeeRequired));
    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Confirm transaction fee"), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    *payFee = (retval == QMessageBox::Yes);
}

void BitcoinGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
    if(!clientModel->inInitialBlockDownload())
    {
        // On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));
        if (convertmode == 0)
        {
            notificator->notify(Notificator::Information,
                                (amount)<0 ? tr("Sent transaction") :
                                             tr("Incoming transaction"),
                                tr("Date: %1\n"
                                   "Amount: %2\n"
                                   "Type: %3\n"
                                   "Address: %4\n")
                                .arg(date)
                                .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                                .arg(type)
                                .arg(address), icon);
        }
        if (convertmode == 1)
        {
            notificator->notify(Notificator::Information,
                                (amount)<0 ? tr("Sent transaction") :
                                             tr("Incoming transaction"),
                                tr("Date: %1\n"
                                   "Amount: %2\n"
                                   "Type: %3\n"
                                   "Address: %4\n")
                                .arg(date)
                                .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), dollarg.toDouble()*amount, true))
                                .arg(type)
                                .arg(address), icon);

        }
        if (convertmode == 2)
        {
            notificator->notify(Notificator::Information,
                                (amount)<0 ? tr("Sent transaction") :
                                             tr("Incoming transaction"),
                                tr("Date: %1\n"
                                   "Amount: %2\n"
                                   "Type: %3\n"
                                   "Address: %4\n")
                                .arg(date)
                                .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), bitcoing.toDouble()*amount, true))
                                .arg(type)
                                .arg(address), icon);
        }
    }
}

void BitcoinGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);
    actionConvertIcon->setEnabled(true);
    actionConvertIcon->setVisible(true);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    connect(actionConvertIcon, SIGNAL(triggered()), this, SLOT(sConvert()));
    exportAction->setVisible(false);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    wId->raise();
}

void BitcoinGUI::gotoPoolBrowser()
{
    poolAction->setChecked(true);
    centralWidget->setCurrentWidget(poolBrowser);
    exportAction->setEnabled(false);
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    connect(actionConvertIcon, SIGNAL(triggered()), this, SLOT(sConvert()));
    exportAction->setVisible(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    wId->raise();
}

void BitcoinGUI::gotoSettingsPage()
{
    settingsAction->setChecked(true);
    centralWidget->setCurrentWidget(settingsPage);
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    connect(actionConvertIcon, SIGNAL(triggered()), this, SLOT(sConvert()));
    exportAction->setEnabled(false);
    exportAction->setVisible(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    wId->raise();
}

void BitcoinGUI::gotoBlockBrowser()
{
    blockAction->setChecked(true);
    centralWidget->setCurrentWidget(blockBrowser);
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    connect(actionConvertIcon, SIGNAL(triggered()), this, SLOT(sConvert()));
    exportAction->setVisible(false);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    wId->raise();
}

void BitcoinGUI::gotoStatisticsPage()
{
    statisticsAction->setChecked(true);
    centralWidget->setCurrentWidget(statisticsPage);
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    connect(actionConvertIcon, SIGNAL(triggered()), this, SLOT(sConvert()));
    exportAction->setVisible(false);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    wId->raise();
}

void BitcoinGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);
    convertmode= 0;
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    exportAction->setEnabled(true);
    exportAction->setVisible(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
    wId->raise();
}

void BitcoinGUI::gotoAddressBookPage()
{
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);
    convertmode = 0;
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    exportAction->setEnabled(true);
    exportAction->setVisible(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
    wId->raise();
}

void BitcoinGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);
    convertmode = 0;
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    exportAction->setEnabled(true);
    exportAction->setVisible(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
    wId->raise();
}

void BitcoinGUI::gotoSendCoinsPage()
{
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);
    convertmode = 0;
    actionConvertIcon->setEnabled(false);
    actionConvertIcon->setVisible(false);
    disconnect(actionConvertIcon, SIGNAL(triggered()), 0, 0);
    exportAction->setEnabled(false);
    exportAction->setVisible(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    wId->raise();
}

void BitcoinGUI::gotoSignMessageTab(QString addr)
{
    // call show() in showTab_SM()
    signVerifyMessageDialog->showTab_SM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void BitcoinGUI::gotoVerifyMessageTab(QString addr)
{
    // call show() in showTab_VM()
    signVerifyMessageDialog->showTab_VM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);
}

void BitcoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        int nValidUrisFound = 0;
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            if (sendCoinsPage->handleURI(uri.toString()))
                nValidUrisFound++;
        }

        // if valid URIs were found
        if (nValidUrisFound)
            gotoSendCoinsPage();
        else
            notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid CAIx address or malformed URI parameters."));
    }

    event->acceptProposedAction();
}

void BitcoinGUI::handleURI(QString strURI)
{
    // URI has to be valid
    if (sendCoinsPage->handleURI(strURI))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
    }
    else
    {
        notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid CAIx address or malformed URI parameters."));
    }
}

void BitcoinGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>not encrypted</b>, go to actions to encrypt your wallet"));
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        changePassphraseAction->setVisible(false);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->hide();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->hide();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}

void BitcoinGUI::encryptWallet(bool status)
{
    if(!walletModel)
    {
        return;
    }
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt:
                                     AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void BitcoinGUI::backupWallet()
{
    QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty())
    {
        if(!walletModel->backupWallet(filename))
        {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void BitcoinGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void BitcoinGUI::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog::Mode mode = sender() == unlockWalletAction ?
              AskPassphraseDialog::UnlockStaking : AskPassphraseDialog::Unlock;
        AskPassphraseDialog dlg(mode, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void BitcoinGUI::lockWallet()
{
    if(!walletModel)
    {
        return;
    }

    walletModel->setWalletLocked(true);
}

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void BitcoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void BitcoinGUI::mousePressEvent(QMouseEvent *event) {
    m_nMouseClick_X_Coordinate = event->x();
    m_nMouseClick_Y_Coordinate = event->y();
}

void BitcoinGUI::mouseMoveEvent(QMouseEvent *event) {
    move(event->globalX()-m_nMouseClick_X_Coordinate,event->globalY()-m_nMouseClick_Y_Coordinate);
}


void BitcoinGUI::updateStakingIcon()
{
    uint64_t nMinWeight = 0, nMaxWeight = 0, nWeight = 0;
    if (pwalletMain)
        pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);

    if (nLastCoinStakeSearchInterval && nWeight)
    {
        uint64_t nNetworkWeight = GetPoSKernelPS();
        unsigned nEstimateTime = nTargetSpacing * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
            text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
            text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
            text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
            text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }

        labelStakingIcon->setPixmap(QIcon(":/icons/staking_on").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        labelStakingIcon->setToolTip(tr("Staking.<br>Your weight is %1<br>Network weight is %2<br>Expected time to earn reward is %3").arg(nWeight).arg(nNetworkWeight).arg(text));
        labelStakingIcon->setStyleSheet("QToolBar QWidget {color: white;}");
    }
    else
    {
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_off").pixmap(STATUSBAR_wICONSIZE,STATUSBAR_hICONSIZE));
        if (pwalletMain && pwalletMain->IsLocked())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is locked"));
        else if (vNodes.empty())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is offline"));
        else if (IsInitialBlockDownload())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is syncing"));
        else if (!nWeight)
            labelStakingIcon->setToolTip(tr("Not staking because you don't have mature coins"));
        else
            labelStakingIcon->setToolTip(tr("Not staking"));
    }
}
