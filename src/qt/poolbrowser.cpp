#include "poolbrowser.h"
#include "ui_poolbrowser.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"
#include "clientmodel.h"
#include "bitcoinrpc.h"

#include <QDesktopServices>
#include <sstream>
#include <string>

using namespace json_spirit;

#define QSTRING_DOUBLE(var) QString::number(var, 'f', 8)

// Markets pages
const QString kBittrexPage = "https://www.bittrex.com/Market/Index?MarketName=BTC-CAIX";
const QString kMintPalPage = "https://www.mintpal.com/market/CAIx/BTC";
const QString kCryptsyPage = "https://www.cryptsy.com/markets/view/221";

// Bitcoin to USD
const QString kCurrencyUSDUrl    = "http://api.coindesk.com/v1/bpi/currentprice.json";

// Bittrex API urls
const QString kBittrexSummaryUrl        = "http://bittrex.com/api/v1.1/public/getmarketsummary?market=btc-caix";
const QString kBittrexOrdersUrl         = "http://bittrex.com/api/v1.1/public/getorderbook?market=BTC-CAIX&type=both&depth=50";
const QString kBittrexHistoryUrl        = "http://bittrex.com/api/v1.1/public/getmarkethistory?market=BTC-CAIX&count=100";

// MintPal API urls
const QString kMintPalSummaryUrl    = "https://api.mintpal.com/v2/market/stats/CAIx/BTC";
const QString kMintPalOrdersUrl     = "https://api.mintpal.com/v2/market/orders/CAIx/BTC/ALL";
const QString kMintPalHistoryUrl    = "https://api.mintpal.com/v2/market/trades/CAIx/BTC";

// Cryptsy API urls
const QString kCryptsySummaryUrl    = "http://pubapi.cryptsy.com/api.php?method=singlemarketdata&marketid=221";
const QString kCryptsyOrdersUrl     = "http://pubapi.cryptsy.com/api.php?method=singleorderdata&marketid=221";

QString bitcoinp = "";
double bitcoinToUSD;
double lastuG;
QString bitcoing;
QString dollarg;
int mode=1;
QString lastp = "";
QString askp = "";
QString bidp = "";
QString highp = "";
QString lowp = "";
QString volumebp = "";
QString volumesp = "";
QString bop = "";
QString sop = "";
QString lastp2 = "";
QString askp2 = "";
QString bidp2 = "";
QString highp2 = "";
QString lowp2 = "";
QString volumebp2 = "";
QString volumesp2 = "";
QString bop2 = "";
QString sop2 = "";

double lastBittrex = 0.0;
double askBittrex = 0.0;
double bidBittrex = 0.0;
double volumeSCBittrex = 0.0;
double volumeBTCBittrex = 0.0;
double highBittrex = 0.0;
double lowBittrex = 0.0;
double changeBittrex = 0.0;

double lastMintPal  = 0.0;
double askMintPal   = 0.0;
double bidMintPal   = 0.0;
double volumeMintPal = 0.0;
double highMintPal  = 0.0;
double lowMintPal   = 0.0;
double changeMintPal = 0.0;

double lastCryptsy  = 0.0;
double askCryptsy   = 0.0;
double bidCryptsy   = 0.0;
double volumeCryptsy = 0.0;
double highCryptsy  = 0.0;
double lowCryptsy   = 0.0;


PoolBrowser::PoolBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PoolBrowser)
{
    ui->setupUi(this);
    ui->sellQuantityTable->header()->setResizeMode(QHeaderView::Fixed);
    ui->buyQuantityTable->header()->setResizeMode(QHeaderView::Fixed);
    ui->sellTableCryptsy->header()->setResizeMode(QHeaderView::Fixed);
    ui->buyTableCryptsy->header()->setResizeMode(QHeaderView::Fixed);
    ui->tradesCryptsyTable->header()->setResizeMode(QHeaderView::Fixed);
    ui->tradesTableBittrex->header()->setResizeMode(QHeaderView::Fixed);
    setFixedSize(400, 420);

    this->setupBittrexGraphs();
    //this->setupMintPalGraphs();
    this->setupCryptsyGraphs();

    QObject::connect(&m_nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseNetworkResponse(QNetworkReply*)));
    connect(ui->egal, SIGNAL(pressed()), this, SLOT(egaldo()));

    updateBitcoinPrice();

    this->setupBittrexTabSlots();
    //this->setupMintPalTabSlots();
    this->setupCryptsyTabSlots();

    this->downloadAllMarketsData();
}

void PoolBrowser::setupBittrexGraphs()
{
    ui->pricesBittrexPlot->addGraph();
}

void PoolBrowser::setupBittrexTabSlots()
{
    connect(ui->refreshBittrexButton, SIGNAL(pressed()), this, SLOT(updateBitcoinPrice()));
    connect(ui->refreshBittrexButton, SIGNAL(pressed()), this, SLOT(downloadBittrexMarketData()));
    connect(ui->bittrexButton, SIGNAL(pressed()), this, SLOT(openBittrexPage()));
}

//void PoolBrowser::setupMintPalGraphs()
//{
//    ui->priceMintPalPlot->addGraph();
//    ui->orderMintPalPlot->addGraph();
//    ui->orderMintPalPlot->addGraph();
//}

void PoolBrowser::updateBitcoinPrice()
{
    this->getRequest(kCurrencyUSDUrl);
}

//void PoolBrowser::setupMintPalTabSlots()
//{
//    connect(ui->refreshMintPalButton, SIGNAL(pressed()), this, SLOT(downloadMintPalMarketData()));
//    connect(ui->refreshMintPalButton, SIGNAL(pressed()), this, SLOT(updateBitcoinPrice()));
//    connect(ui->mintPalButton, SIGNAL(pressed()), this, SLOT(openMintPalPage()));
//}

void PoolBrowser::setupCryptsyGraphs()
{
    ui->priceCryptsyPlot->addGraph();
}

void PoolBrowser::setupCryptsyTabSlots()
{
    connect(ui->refreshCryptsyButton, SIGNAL(pressed()), this, SLOT(updateBitcoinPrice()));
    connect(ui->refreshCryptsyButton, SIGNAL(pressed()), this, SLOT(downloadCryptsyMarketData()));
    connect(ui->cryptsyButton, SIGNAL(pressed()), this, SLOT(openCryptsyPage()));
}

void PoolBrowser::egaldo()
{
    QString temps = ui->egals->text();
    double totald =  lastBittrex * bitcoinToUSD * temps.toDouble();
    double totaldq = lastBittrex * temps.toDouble();
    ui->egald->setText(QString::number(totald) + " $ / "+QString::number(totaldq)+" BTC");
}

void PoolBrowser::getRequest( const QString &urlString )
{
    QUrl url ( urlString );
    QNetworkRequest req ( url );
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    req.setSslConfiguration(config);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    m_nam.get(req);
}

void PoolBrowser::parseNetworkResponse(QNetworkReply *finished )
{
    QUrl requestUrl = finished->url();

    if ( finished->error() != QNetworkReply::NoError )
    {
        // A communication error has occurred
        emit networkError( finished->error() );
        return;
    }


    if (requestUrl == kBittrexSummaryUrl)
    {
        this->parseBittrexSummary(finished);
    }
    else if (requestUrl == kCurrencyUSDUrl)
    {
        this->parseCurrencyUSD(finished);
    }
    else if (requestUrl == kBittrexOrdersUrl)
    {
        this->parseBittrexOrders(finished);
    }
    else if (requestUrl == kBittrexHistoryUrl)
    {
        this->parseBittrexHistory(finished);
    }
//    else if (requestUrl == kMintPalSummaryUrl)
//    {
//        this->parseMintPalSummary(finished);
//    }
//    else if (requestUrl == kMintPalOrdersUrl)
//    {
//        this->parseMintPalOrders(finished);
//    }
//    else if (requestUrl == kMintPalHistoryUrl)
//    {
//        this->parseMintPalHistory(finished);
//    }
    else if (requestUrl == kCryptsySummaryUrl)
    {
        this->parseCryptsySummary(finished);
    }
    else if (requestUrl == kCryptsyOrdersUrl)
    {
        this->parseCryptsyOrders(finished);
    }

    finished->deleteLater();
}

void PoolBrowser::parseCurrencyUSD(QNetworkReply *reply)
{
    // QNetworkReply is a QIODevice. So we read from it just like it was a file
    QString data = reply->readAll();
    QString bitcoin;

    QJsonParseError jsonParseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);
    if (jsonResponse.isObject()) {

        QJsonObject mainObject = jsonResponse.object();

        if (mainObject.contains("bpi")) {
            QJsonObject bpiObject = mainObject["bpi"].toObject();
            QJsonObject USDObject = bpiObject["USD"].toObject();
            bitcoin = USDObject["rate"].toString();
            bitcoinToUSD = bitcoin.toDouble();
        }
    }

    if(bitcoin > bitcoinp) {
        ui->bitcoin->setText("<b><font color=\"green\">" + bitcoin + " $</font></b>");
    } else if (bitcoin < bitcoinp) {
        ui->bitcoin->setText("<b><font color=\"red\">" + bitcoin + " $</font></b>");
    } else {
        ui->bitcoin->setText(bitcoin + " $");
    }

    bitcoinp = bitcoin;
}

void PoolBrowser::parseBittrexSummary(QNetworkReply *replay)
{
    QString data = replay->readAll();

    qDebug() << "BittrexSummary response:" << data;

    QJsonParseError jsonParseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);
    if (jsonResponse.isObject()) {

       QJsonObject mainObject = jsonResponse.object();
       if (!mainObject.contains("success")
               && !mainObject["success"].toBool()) {
           return;
       }

       if (mainObject.contains("result")) {
           QJsonArray resultArray = mainObject["result"].toArray();
           foreach (const QJsonValue &value, resultArray) {
               QJsonObject marketObject = value.toObject();
               if (marketObject.contains("MarketName")
                   && marketObject["MarketName"].toString() == "BTC-CAIX") {

                   double dollargD = lastBittrex * bitcoinToUSD;
                   dollarg = QSTRING_DOUBLE(dollargD);
                   bitcoing = QSTRING_DOUBLE(lastBittrex);

                   // Updating summary labels
                   highBittrex = this->refreshDoubleVarUsingField(marketObject, "High",
                                                                  highBittrex, ui->highBittrex);
                   lowBittrex = this->refreshDoubleVarUsingField(marketObject, "Low",
                                                                 lowBittrex, ui->lowBittrex);

                   this->refreshDoubleVarAsBTCUsingField(marketObject, "BaseVolume",
                                                         volumeBTCBittrex, ui->volumeBTCBittrex);
                   volumeBTCBittrex = this->refreshDoubleVarAsUSDUsingField(marketObject, "BaseVolume",
                                                                            volumeBTCBittrex, ui->volumeUSDBittrex);

                   this->refreshDoubleVarAsBTCUsingField(marketObject, "Last",
                                                         lastBittrex, ui->lastBTCBittrex);
                   lastBittrex = this->refreshDoubleVarAsUSDUsingField(marketObject, "Last",
                                                                       lastBittrex, ui->lastUSDBittrex);

//                   volumeSCBittrex = this->refreshDoubleVarUsingField(marketObject, "Volume",
//                                                                      volumeSCBittrex, ui->volumeSCBittrex);

                   this->refreshDoubleVarAsBTCUsingField(marketObject, "Bid",
                                                         bidBittrex, ui->bidBTCBittrex);
                   bidBittrex = this->refreshDoubleVarAsUSDUsingField(marketObject, "Bid",
                                                                      bidBittrex, ui->bidUSDBittrex);

                   qDebug() << "Refreshing ask:";
                   this->refreshDoubleVarAsBTCUsingField(marketObject, "Ask",
                                                         askBittrex, ui->askBTCBittrex);
                   askBittrex = this->refreshDoubleVarAsUSDUsingField(marketObject, "Ask",
                                                                      askBittrex, ui->askUSDBittrex);

//                   if (marketObject.contains("OpenBuyOrders")) {
//                       ui->openBuyersBittrex->setText(QSTRING_DOUBLE(marketObject["OpenBuyOrders"].toDouble()));
//                   }
//                   if (marketObject.contains("OpenSellOrders")) {
//                       ui->openSellersBittrex->setText(QSTRING_DOUBLE(marketObject["OpenSellOrders"].toDouble()));
//                   }

                   if (marketObject.contains("PrevDay")
                           && marketObject.contains("Last")) {
                       double yesterday = marketObject["PrevDay"].toDouble();
                       double today = marketObject["Last"].toDouble();

                       double change;
                       if (today > yesterday) {
                           change = ( (today - yesterday) / today ) * 100;
                           this->setGreenTextForLabel("+" + QSTRING_DOUBLE(change) + "%", ui->yest);
                       } else {
                           change = ( (yesterday - today) / yesterday) * 100;
                           this->setRedTextForLabel("-" + QSTRING_DOUBLE(change) + "%", ui->yest);
                       }
                   }
               }
           }
       }

    } else {
        qWarning() << "Parsing Bittrex summary failed with error: " << jsonParseError.errorString();
    }
}

void PoolBrowser::parseBittrexOrders(QNetworkReply *replay)
{
    QString data = replay->readAll();

    qDebug() << "BittrexSummary response:" << data;

    QJsonParseError jsonParseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);
    if (jsonResponse.isObject()) {

       QJsonObject mainObject = jsonResponse.object();
       if (!mainObject.contains("success")
               && !mainObject["success"].toBool()) {
           return;
       }

       if (mainObject.contains("result")) {
           QJsonObject resultObject = mainObject["result"].toObject();

           QVector<double> volumeSell(50), satoshiSell(50);
           QVector<double> volumeBuy(50), satoshiBuy(50);

           if (resultObject.contains("buy")) {

               ui->buyQuantityTable->clear();

               QJsonArray buyArray = resultObject["buy"].toArray();

               int i = 0;
               double cumulation = 0.0;
               foreach (const QJsonValue &order, buyArray) {
                    QJsonObject orderObject = order.toObject();
                    QTreeWidgetItem *orderItem = new QTreeWidgetItem();
                    double rate = 0;
                    double quantity = 0;

                    if (orderObject.contains("Rate")) {
                        rate = orderObject["Rate"].toDouble();
                        orderItem->setText(0, QSTRING_DOUBLE(rate));

                        double satoshi = rate * 100000000;

                        satoshiBuy[i] = satoshi;
                        volumeBuy[i] = cumulation;
                    }
                    if (orderObject.contains("Quantity")) {
                        quantity = orderObject["Quantity"].toDouble();
                        orderItem->setText(1, QSTRING_DOUBLE(quantity));
                        cumulation += quantity;
                    }

                    if (orderObject.contains("Rate") && orderObject.contains("Quantity"))
                    {
                        double total = rate * quantity;
                        orderItem->setText(2, QSTRING_DOUBLE(total));
                    }

                    ui->buyQuantityTable->addTopLevelItem(orderItem);
                }
           }

           if (resultObject.contains("sell")) {

               ui->sellQuantityTable->clear();
               ui->sellQuantityTable->sortByColumn(0, Qt::AscendingOrder);

               QJsonArray sellArray = resultObject["sell"].toArray();

               int i = 0;
               double cumulation = 0.0;
               foreach (const QJsonValue &order, sellArray) {
                    QJsonObject orderObject = order.toObject();
                    QTreeWidgetItem *orderItem = new QTreeWidgetItem();
                    double rate = 0;
                    double quantity = 0;

                    if (orderObject.contains("Rate")) {
                        rate = orderObject["Rate"].toDouble();
                        orderItem->setText(0, QSTRING_DOUBLE(rate));

                        double satoshi = rate * 100000000;

                        satoshiSell[i] = satoshi;
                        volumeSell[i] = cumulation;
                    }
                    if (orderObject.contains("Quantity")) {
                        quantity = orderObject["Quantity"].toDouble();
                        orderItem->setText(1, QSTRING_DOUBLE(quantity));

                        cumulation += quantity;
                    }

                    if (orderObject.contains("Rate") && orderObject.contains("Quantity"))
                    {
                        double total = rate * quantity;
                        orderItem->setText(2, QSTRING_DOUBLE(total));
                    }

                    ui->sellQuantityTable->addTopLevelItem(orderItem);
                }
           }
       }

    } else {
        qWarning() << "Parsing Bittrex summary failed with error: " << jsonParseError.errorString();
    }
}

void PoolBrowser::parseBittrexHistory(QNetworkReply *replay)
{
    QString data = replay->readAll();

    qDebug() << "BittrexSummary response:" << data;

    QJsonParseError jsonParseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);
    if (jsonResponse.isObject()) {

       QJsonObject mainObject = jsonResponse.object();
       if (!mainObject.contains("success")
               && !mainObject["success"].toBool()) {
           return;
       }

       if (mainObject.contains("result")) {
           QJsonArray resultArray = mainObject["result"].toArray();

           ui->tradesTableBittrex->clear();
           ui->tradesTableBittrex->setColumnWidth(0,  60);
           ui->tradesTableBittrex->setColumnWidth(1,  100);
           ui->tradesTableBittrex->setColumnWidth(2,  100);
           ui->tradesTableBittrex->setColumnWidth(3,  100);
           ui->tradesTableBittrex->setColumnWidth(4,  180);
           ui->tradesTableBittrex->setColumnWidth(5,  100);
           ui->tradesTableBittrex->sortByColumn(4);

           QVector<double> count(50), prices(50);

           int i = 0;
           foreach (const QJsonValue &tradeValue, resultArray) {

               QJsonObject tradeObject = tradeValue.toObject();
               QTreeWidgetItem *tradeItem = new QTreeWidgetItem();

               if (tradeObject.contains("OrderType")) {
                   QString typeStr = tradeObject["OrderType"].toString();
                   tradeItem->setText(0, typeStr);
               }
               if (tradeObject.contains("Quantity")) {
                   double quantity = tradeObject["Quantity"].toDouble();
                   tradeItem->setText(2, QSTRING_DOUBLE(quantity));
               }
               if (tradeObject.contains("Price")) {
                   double rate = tradeObject["Price"].toDouble();
                   tradeItem->setText(1, QSTRING_DOUBLE(rate));

                   count[i]  = resultArray.count() - i;
                   prices[i] = rate * 100000000;
               }
               if (tradeObject.contains("Total")) {
                   double total = tradeObject["Total"].toDouble();
                   tradeItem->setText(3, QSTRING_DOUBLE(total));
               }
               if (tradeObject.contains("TimeStamp")) {
                   QString timeStampStr = tradeObject["TimeStamp"].toString();
                   tradeItem->setText(4, timeStampStr);
               }
               if (tradeObject.contains("Id")) {
                   double id = tradeObject["Id"].toDouble();
                   tradeItem->setText(5, QSTRING_DOUBLE(id));
               }

               ui->tradesTableBittrex->addTopLevelItem(tradeItem);
               i++;
           }
           this->updatePricesPlot(count, prices, ui->pricesBittrexPlot);
       }

    } else {
        qWarning() << "Parsing Bittrex summary failed with error: " << jsonParseError.errorString();
    }
}

//void PoolBrowser::parseMintPalSummary(QNetworkReply *replay)
//{
//    QString data = replay->readAll();

//    qDebug() << "MintPalSummary response:" << data;

//    QJsonParseError jsonParseError;
//    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);

//    if (jsonResponse.isObject()) {

//        QJsonObject mainObject = jsonResponse.object();

//        if (!mainObject.contains("data")) {
//            return;
//        }

//        QJsonObject dataObject = mainObject["data"].toObject();

//        // Updating last labels
//        if (dataObject.contains("last_price")) {

//            // BTC
//            QString lastPriceStr = dataObject["last_price"].toString();
//            qDebug() << "Last Price is " << dataObject["last_price"].toString();
//            double lastPriceDouble = lastPriceStr.toDouble();
//            this->setColoredTextBasedOnValues(lastPriceStr + " BTC", ui->lastBTCMintPalLabel,
//                                              lastMintPal, lastPriceDouble);
//            // USD
//            double lastPriceUSD = lastPriceDouble * bitcoinToUSD;
//            QString lastPriceUSDStr = QSTRING_DOUBLE(lastPriceUSD);
//            double prevPriceUSD = lastMintPal * bitcoinToUSD;
//            this->setColoredTextBasedOnValues(lastPriceUSDStr + " $", ui->lastUSDMintPalLabel,
//                                              prevPriceUSD, lastPriceUSD);

//            lastMintPal = lastPriceDouble;

//            // Saving variables needed to calculate form CAIx to BC and USD
//            bitcoing = lastPriceStr;
//            dollarg = lastPriceUSDStr;
//            lastuG = lastPriceUSD;
//        }

//        // Updating ask labels
//        if (dataObject.contains("top_ask")) {
//            // BTC
//            QString askStr = dataObject["top_ask"].toString();
//            double askDouble = askStr.toDouble();
//            this->setColoredTextBasedOnValues(askStr + " BTC", ui->askBTCMintPalLabel,
//                                              askMintPal, askDouble);
//            // USD
//            double askUSD = askDouble * bitcoinToUSD;
//            double prevAskUSD = askMintPal * bitcoinToUSD;
//            this->setColoredTextBasedOnValues(QSTRING_DOUBLE(askUSD) + " $", ui->askUSDMintPalLabel,
//                                              prevAskUSD, askUSD);

//            askMintPal = askDouble;
//        }

//        // Updating bid labels
//        if (dataObject.contains("top_bid")) {
//            // BTC
//            QString bidStr = dataObject["top_bid"].toString();
//            double bidDouble = bidStr.toDouble();
//            this->setColoredTextBasedOnValues(bidStr + " BTC", ui->bidBTCMintPalLabel,
//                                              bidMintPal, bidDouble);
//            // USD
//            double bidUSD = bidDouble * bitcoinToUSD;
//            double prevBidUSD = bidMintPal * bitcoinToUSD;
//            this->setColoredTextBasedOnValues(QSTRING_DOUBLE(bidUSD) + " $", ui->bidUSDMintPalLabel,
//                                              prevBidUSD, bidUSD);

//            bidMintPal = bidDouble;
//        }

//        // Updating volume labels
//        if (dataObject.contains("24hvol")) {
//            // BTC
//            QString volumeStr = dataObject["24hvol"].toString();
//            double volumeDouble = volumeStr.toDouble();
//            this->setColoredTextBasedOnValues(volumeStr, ui->volumesBTCMintPalLabel,
//                                              volumeMintPal, volumeDouble);

//            // USD
//            double volumeUSD = volumeDouble * bitcoinToUSD;
//            double prevVolumeUSD = volumeMintPal * bitcoinToUSD;
//            this->setColoredTextBasedOnValues(QSTRING_DOUBLE(volumeUSD) + " $", ui->volumeUSDMintPalLabel,
//                                              prevVolumeUSD, volumeUSD);

//            volumeMintPal = volumeDouble;
//        }

//        // Updating high label
//        if (dataObject.contains("24hhigh")) {
//            QString highStr = dataObject["24hhigh"].toString();
//            double highDouble = highStr.toDouble();
//            this->setColoredTextBasedOnValues(highStr, ui->highMintPalLabel,
//                                              highMintPal, highDouble);
//            highMintPal = highDouble;
//        }

//        // Updating low label
//        if (dataObject.contains("24hlow")) {
//            QString lowStr = dataObject["24hlow"].toString();
//            double lowDouble = lowStr.toDouble();
//            this->setColoredTextBasedOnValues(lowStr, ui->lowMintPalLabel,
//                                              lowMintPal, lowDouble);
//            lowMintPal = lowDouble;
//        }

//        if (dataObject.contains("change")) {
//            QString change = dataObject["change"].toString();
//            double changeDouble = change.toDouble();
//            this->setColoredTextBasedOnValues(change + "%", ui->yestMintpal,
//                                              changeMintPal, changeDouble);
//            changeMintPal = changeDouble;
//        }
//    } else {
//        qWarning() << "Parsing MintPal summary failed with error: " << jsonParseError.errorString();
//    }
//}

//void PoolBrowser::parseMintPalOrders(QNetworkReply *replay)
//{
//    QString data = replay->readAll();
//    qDebug() << "MintPalOrders response" + data;

//    QJsonParseError jsonParseError;
//    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);

//    if (jsonResponse.isObject()) {

//        QJsonObject mainObject = jsonResponse.object();
//        if (!mainObject.contains("data")) {
//            return;
//        }

//        QJsonArray dataArray = mainObject["data"].toArray();

//        ui->buyTableMintPal->clear();
//        ui->sellTableMintPal->clear();

//        QVector<double> volumeSell(50), satoshiSell(50);
//        QVector<double> volumeBuy(50), satoshiBuy(50);

//        foreach (const QJsonValue &value, dataArray) {

//            QJsonObject obj = value.toObject();
//            if (obj.contains("type") && obj.contains("orders"))
//            {
//                QString typeString = obj["type"].toString();
//                QJsonArray orders = obj["orders"].toArray();

//                QTreeWidget *table = NULL;
//                QVector<double> *volumeVector = NULL, *satoshiVector = NULL;
//                if (typeString == "buy") {
//                    table = ui->buyTableMintPal;
//                    volumeVector = &volumeBuy;
//                    satoshiVector = &satoshiBuy;
//                }
//                else if (typeString == "sell") {
//                    table = ui->sellTableMintPal;
//                    volumeVector = &volumeSell;
//                    satoshiVector = &satoshiSell;
//                }
//                else {
//                    continue;
//                }

//                int i = 0;
//                double cumulation = 0.0;
//                foreach (const QJsonValue &orderValue, orders) {

//                    QJsonObject order = orderValue.toObject();
//                    QTreeWidgetItem *orderItem = new QTreeWidgetItem();

//                    if (order.contains("price")) {
//                        QString priceStr = order["price"].toString();
//                        orderItem->setText(0, priceStr);

//                        double satoshi = priceStr.toDouble() * 100000000;

//                        (*satoshiVector)[i] = satoshi;
//                        (*volumeVector)[i] = cumulation;
//                    }
//                    if (order.contains("amount")) {
//                        QString amountString = order["amount"].toString();
//                        orderItem->setText(1, amountString);

//                        cumulation += amountString.toDouble();
//                    }
//                    if (order.contains("total")) {
//                        orderItem->setText(2, order["total"].toString());
//                    }
//                    table->addTopLevelItem(orderItem);
//                    i++;
//                }
//            }
//        }

//       this->updateVolumeSatoshiPlot(satoshiSell, satoshiBuy, volumeSell, volumeBuy, ui->orderMintPalPlot);

//    } else {
//        qWarning() << "Parsing MintPal orders failed with error: " << jsonParseError.errorString();
//    }
//}

//void PoolBrowser::parseMintPalHistory(QNetworkReply *replay)
//{
//    QString data = replay->readAll();
//    qDebug() << "MintPal history response: " + data;

//    QJsonParseError jsonParseError;
//    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);

//    if (jsonResponse.isObject()) {

//        ui->tradesMintPalTable->clear();

//        QJsonObject mainObject = jsonResponse.object();
//        if (!mainObject.contains("data")) {
//            return;
//        }

//        ui->tradesMintPalTable->sortByColumn(5, Qt::DescendingOrder);

//        QVector<double> count(100), prices(100);

//        QJsonArray dataArray = mainObject["data"].toArray();

//        int i = 0;

//        foreach (const QJsonValue &value, dataArray) {

//            QJsonObject tradeObject = value.toObject();

//            QTreeWidgetItem *tradeItem = new QTreeWidgetItem();

//            if (tradeObject.contains("type")) {
//                tradeItem->setText(0, tradeObject["type"].toString());
//            }
//            if (tradeObject.contains("price")) {

//                QString priceString = tradeObject["price"].toString();
//                tradeItem->setText(1, priceString);

//                count[i]  = dataArray.count() - i;
//                prices[i] = priceString.toDouble() * 100000000;
//            }
//            if (tradeObject.contains("amount")) {
//                tradeItem->setText(2, tradeObject["amount"].toString());
//            }
//            if (tradeObject.contains("total")) {
//                tradeItem->setText(3, tradeObject["total"].toString());
//            }
//            if (tradeObject.contains("time")) {
//                double tradeTimeStamp = tradeObject["time"].toString().toDouble();

//                QDateTime timestamp;
//                timestamp.setTime_t(tradeTimeStamp);

//                tradeItem->setText(4, timestamp.toString(Qt::SystemLocaleShortDate));
//            }

//            ui->tradesMintPalTable->addTopLevelItem(tradeItem);
//            i++;
//        }
//        this->updateMintPalPricesPlot(count, prices);

//    } else {
//        qWarning() << "Parsing MintPal history failed with error: " << jsonParseError.errorString();
//    }
//}

void PoolBrowser::parseCryptsySummary(QNetworkReply *replay)
{
    QString data = replay->readAll();
    qDebug() << "Cryptsy summary response: " + data;

    QJsonParseError jsonParseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);

    if (jsonResponse.isObject()) {
        ui->tradesCryptsyTable->clear();
        ui->tradesCryptsyTable->setColumnWidth(0,  60);
        ui->tradesCryptsyTable->setColumnWidth(1,  100);
        ui->tradesCryptsyTable->setColumnWidth(2,  100);
        ui->tradesCryptsyTable->setColumnWidth(3,  100);
        ui->tradesCryptsyTable->setColumnWidth(4,  180);
        ui->tradesCryptsyTable->setColumnWidth(5,  100);
        ui->tradesCryptsyTable->sortByColumn(4);

        QJsonObject mainObject = jsonResponse.object();

        if (mainObject.contains("return")) {
            QJsonObject returnObject = mainObject["return"].toObject();
            if (returnObject.contains("markets")) {
                QJsonObject marketsObject = returnObject["markets"].toObject();
                if (marketsObject.contains("CAIx")) {

                    QJsonObject caixObject = marketsObject["CAIx"].toObject();

                    // Updating summary labels
                    this->refreshStringVarAsBTCUsingField(caixObject, "lasttradeprice",
                                                    lastCryptsy, ui->lastBTCCryptsyLabel);


                    lastCryptsy = this->refreshStringVarAsUSDUsingField(caixObject, "lasttradeprice",
                                                                  lastCryptsy, ui->lastUSDCryptsyLabel);

                    volumeCryptsy = this->refreshStringVarUsingField(caixObject, "volume",
                                               volumeCryptsy, ui->volumesBTCCryptsyLabel);

                    double high = -999999999;
                    double low  =  999999999;
                    int i = 0;
                    if (caixObject.contains("recenttrades")) {

                        //ui->tradesCryptsyTable->clear();

                        QJsonArray tradesArray = caixObject["recenttrades"].toArray();

                        QVector<double> count(tradesArray.count()), prices(tradesArray.count());

                        foreach (const QJsonValue &value, tradesArray) {
                            QJsonObject tradeObject = value.toObject();

                            QTreeWidgetItem *tradeItem = new QTreeWidgetItem();

                            if (tradeObject.contains("type")) {
                                tradeItem->setText(0, tradeObject["type"].toString().toUpper());
                            }
                            if (tradeObject.contains("price")) {
                                QString priceString = tradeObject["price"].toString();

                                tradeItem->setText(1, priceString);

                                double price = priceString.toDouble();
                                count[i]  = tradesArray.count() - i;
                                prices[i] = price * 100000000;

                                if (price > high) {
                                    high = price;
                                }
                                if (price < low) {
                                    low = price;
                                }
                            }
                            if (tradeObject.contains("quantity")) {
                                tradeItem->setText(2, tradeObject["quantity"].toString());
                            }
                            if (tradeObject.contains("total")) {
                                tradeItem->setText(3, tradeObject["total"].toString());
                            }
                            if (tradeObject.contains("time")) {
                                tradeItem->setText(4, tradeObject["time"].toString());
                            }

                            ui->tradesCryptsyTable->addTopLevelItem(tradeItem);
                            i++;
                        }
                        this->updateCrypstyPricesPlot(count, prices);
                    }

                    // Updating high / low labels
                    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(high), ui->highCryptsyLabel, highCryptsy, high);
                    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(low), ui->lowCryptsyLabel, lowCryptsy, low);

                    lowCryptsy = low;
                    highCryptsy = high;
                }
            }
        }

    } else {
        qWarning() << "Parsing Cryptsy summary failed with error: " << jsonParseError.errorString();
    }
}

void PoolBrowser::parseCryptsyOrders(QNetworkReply *replay)
{
    QString data = replay->readAll();
    qDebug() << "Cryptsy orders response: " + data;

    QJsonParseError jsonParseError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8(), &jsonParseError);

    if (jsonResponse.isObject()) {
        ui->sellTableCryptsy->clear();
        ui->buyTableCryptsy->clear();

        QJsonObject mainObject = jsonResponse.object();
        if (mainObject.contains("return")) {
            QJsonObject returnObject = mainObject["return"].toObject();
            if (returnObject.contains("CAIx")) {
                QJsonObject caixObject = returnObject["CAIx"].toObject();

                int sellCount = 50, buyCount = 50;
                if (caixObject.contains("sellorders")) {
                    sellCount = caixObject["sellorders"].toArray().count();
                }
                if (caixObject.contains("buyorders")) {
                    buyCount = caixObject["buyorders"].toArray().count();
                }

                QVector<double> satoshiBuy(buyCount), satoshiSell(sellCount);
                QVector<double> volumeBuy(buyCount), volumeSell(sellCount);

                double cumulation = 0.0;
                if (caixObject.contains("sellorders")) {

                    double ask = 9999999;
                    QJsonArray sellArray = caixObject["sellorders"].toArray();
                    //ui->sellTableCryptsy->clear();

                    int i = 0;
                    foreach (const QJsonValue &value, sellArray) {

                        QJsonObject sellObject = value.toObject();
                        QTreeWidgetItem *sellItem = new QTreeWidgetItem();

                        if (sellObject.contains("price")) {
                            QString priceStr = sellObject["price"].toString();
                            double price = priceStr.toDouble();

                            sellItem->setText(0, priceStr);

                            if (price < ask) {
                                ask = price;
                            }

                            double satoshi = priceStr.toDouble() * 100000000;
                            satoshiSell[i] = satoshi;                         
                        }
                        if (sellObject.contains("quantity")) {
                            QString quantityStr = sellObject["quantity"].toString();
                            sellItem->setText(1, quantityStr);

                            cumulation += quantityStr.toDouble();
                        }
                        if (sellObject.contains("total")) {
                            sellItem->setText(2, sellObject["total"].toString());
                        }
                        volumeSell[i] = cumulation;

                        ui->sellTableCryptsy->addTopLevelItem(sellItem);
                        i++;
                    }

                    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(ask) + " BTC", ui->askBTCCryptsyLabel,
                                                      askCryptsy, ask);
                    double askUSD = ask * bitcoinToUSD;
                    double prevAskUSD = askCryptsy * bitcoinToUSD;
                    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(askUSD) + " $", ui->askUSDCryptsyLabel,
                                                      prevAskUSD, askUSD);

                    askCryptsy = ask;
                }

                cumulation = 0.0;
                if (caixObject.contains("buyorders")) {

                    double bid = -999999999;
                    QJsonArray sellArray = caixObject["buyorders"].toArray();
                    //ui->buyTableCryptsy->clear();
                    int i = 0;
                    foreach (const QJsonValue &value, sellArray) {

                        QJsonObject sellObject = value.toObject();
                        QTreeWidgetItem *sellItem = new QTreeWidgetItem();

                        if (sellObject.contains("price")) {
                            QString priceStr = sellObject["price"].toString();
                            double price = priceStr.toDouble();

                            sellItem->setText(0, priceStr);

                            if (price > bid) {
                                bid = price;
                            }

                            double satoshi = priceStr.toDouble() * 100000000;
                            satoshiBuy[i] = satoshi;

                        }
                        if (sellObject.contains("quantity")) {
                            QString quantityStr = sellObject["quantity"].toString();
                            sellItem->setText(1, quantityStr);
                            cumulation += quantityStr.toDouble();
                        }
                        if (sellObject.contains("total")) {
                            sellItem->setText(2, sellObject["total"].toString());
                        }
                        volumeBuy[i] = cumulation;
                        ui->buyTableCryptsy->addTopLevelItem(sellItem);
                    }
                    i++;

                    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(bid) + " BTC", ui->bidBTCCryptsyLabel,
                                                      bidCryptsy, bid);
                    double bidUSD = bid * bitcoinToUSD;
                    double prevBidUSD = bidCryptsy * bitcoinToUSD;
                    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(bidUSD) + " $", ui->bidUSDCryptsyLabel,
                                                      prevBidUSD, bidUSD);

                    bidCryptsy = bid;


                }
            }
        }
    } else {
        qWarning() << "Parsing Cryptsy orders failed with error: " << jsonParseError.errorString();
    }
}

void PoolBrowser::openBittrexPage()
{
    this->openUrl(kBittrexPage);
}

//void PoolBrowser::openMintPalPage()
//{
//    this->openUrl(kMintPalPage);
//}

void PoolBrowser::openCryptsyPage()
{
    this->openUrl(kCryptsyPage);
}

void PoolBrowser::openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void PoolBrowser::downloadAllMarketsData()
{
    this->downloadBittrexMarketData();
    //this->downloadMintPalMarketData();
    this->downloadCryptsyMarketData();
}

void PoolBrowser::downloadBittrexMarketData()
{
    this->getRequest(kBittrexSummaryUrl);
    this->getRequest(kBittrexOrdersUrl);
    this->getRequest(kBittrexHistoryUrl);
}

void PoolBrowser::downloadMintPalMarketData()
{
    this->getRequest(kMintPalSummaryUrl);
    this->getRequest(kMintPalOrdersUrl);
    this->getRequest(kMintPalHistoryUrl);
}

void PoolBrowser::downloadCryptsyMarketData()
{
    this->getRequest(kCryptsySummaryUrl);
    this->getRequest(kCryptsyOrdersUrl);
}

void PoolBrowser::setColoredTextBasedOnValues(const QString &text, QLabel *label, double prevValue, double actualValue)
{
    if(actualValue > prevValue) {
        this->setGreenTextForLabel(text, label);
    } else if (actualValue < prevValue) {
        this->setRedTextForLabel(text, label);
    } else {
        label->setText(text);
    }
}

void PoolBrowser::setRedTextForLabel(const QString &text, QLabel *label)
{
    label->setText("<b><font color=\"red\">" + text + "</font></b>");
}

void PoolBrowser::setGreenTextForLabel(const QString &text, QLabel *label)
{
    label->setText("<b><font color=\"green\">" + text + "</font></b>");
}

void PoolBrowser::refreshValueLabel(double actualValue, QLabel *label, double newDouble, const QString &suffix, QString newStr)
{
    QString uiString = newStr;
    if (suffix.length() > 0)
    {
        uiString += suffix;
    }

    this->setColoredTextBasedOnValues(uiString, label,
                                      actualValue, newDouble);
}

double PoolBrowser::refreshDoubleVarUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label, const QString &suffix)
{
    // Updating high label
    if (jsonObject.contains(fieldName)) {
        double newDouble = jsonObject[fieldName].toDouble();

        qDebug() << newDouble;

        refreshValueLabel(actualValue, label, newDouble, suffix, QSTRING_DOUBLE(newDouble));
        return newDouble;
    } else {
        return actualValue;
    }
}

double PoolBrowser::refreshStringVarUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label, const QString &suffix)
{
    // Updating high label
    if (jsonObject.contains(fieldName)) {

        QString newStr = jsonObject[fieldName].toString();
        double newDouble = newStr.toDouble();

        refreshValueLabel(actualValue, label, newDouble, suffix, newStr);
        return newDouble;
    } else {
        return actualValue;
    }
}

double PoolBrowser::refreshStringVarAsBTCUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label)
{
    return this->refreshStringVarUsingField(jsonObject, fieldName, actualValue, label, " BTC");
}

void PoolBrowser::refreshUSDLabel(QLabel *label, double actualValue, double newDouble)
{
    double newUSD = newDouble * bitcoinToUSD;
    double prevUSD = actualValue * bitcoinToUSD;
    qDebug() << "newUSD " << newUSD << " prevUSD " << prevUSD;
    this->setColoredTextBasedOnValues(QSTRING_DOUBLE(newUSD) + " $", label, prevUSD, newUSD);
}

double PoolBrowser::refreshStringVarAsUSDUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label)
{
    if (jsonObject.contains(fieldName)) {
        double newDouble = jsonObject[fieldName].toString().toDouble();
        refreshUSDLabel(label, actualValue, newDouble);
        return newDouble;
    } else {
        return actualValue;
    }
}

double PoolBrowser::refreshDoubleVarAsBTCUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label)
{
    return this->refreshDoubleVarUsingField(jsonObject, fieldName, actualValue, label, " BTC");
}

double PoolBrowser::refreshDoubleVarAsUSDUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label)
{
    if (jsonObject.contains(fieldName)) {
        double newDouble = jsonObject[fieldName].toDouble();
        refreshUSDLabel(label, actualValue, newDouble);
        return newDouble;
    } else {
        return actualValue;
    }
}

//void PoolBrowser::updateMintPalPricesPlot(const QVector<double> &count, const QVector<double> &prices)
//{
//    this->updatePricesPlot(count, prices, ui->priceMintPalPlot);
//}

void PoolBrowser::updateCrypstyPricesPlot(const QVector<double> &count, const QVector<double> &prices)
{
    this->updatePricesPlot(count, prices, ui->priceCryptsyPlot);
}

void PoolBrowser::updatePricesPlot(const QVector<double> &count, const QVector<double> &prices, QCustomPlot *plot)
{
    double min = 100000000;
    double max = 0;

    QVectorIterator<double> i(prices);
    while (i.hasNext()) {
         double price = i.next();
         if (price > max) {
             max = price;
         } else if (price < min) {
             min = price;
         }
    }

    plot->graph(0)->setData(count, prices);
    plot->graph(0)->setPen(QPen(QColor(34, 177, 76)));
    plot->graph(0)->setBrush(QBrush(QColor(34, 177, 76, 20)));
    plot->xAxis->setRange(0, prices.count());
    plot->yAxis->setRange(min, max);
    plot->replot();
}

double PoolBrowser::getMaxValueFromVector(const QVector<double> &vector) {
    double max = -999999999.0;
    QVectorIterator<double> iterator(vector);
    while (iterator.hasNext()) {
        double value = iterator.next();
        if (value > max) {
            max = value;
        }
    }
    return max;
}

double PoolBrowser::getMinValueFromVector(const QVector<double> &vector) {
    double min = 999999999.0;
    QVectorIterator<double> iterator(vector);
    while (iterator.hasNext()) {
        double value = iterator.next();
        if (value < min) {
            min = value;
        }
    }
    return min;
}

void PoolBrowser::setModel(ClientModel *model)
{
    this->model = model;
}

PoolBrowser::~PoolBrowser()
{
    delete ui;
}
