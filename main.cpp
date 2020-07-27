#include <QApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#define SPO2_SAMPLE_RATE 60 //test 2019 //125

class PatientModel : public QSqlQueryModel
{
public:
    PatientModel()
    {
    }

    // 配置单元格标志
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
//        qDebug()<<index.column()<<index.row();
        Qt::ItemFlags flags = QSqlQueryModel::flags(index);

        if(index.column() == 1 || index.column() == 2)
        {
            flags |= Qt::ItemIsEditable; //flags = flags | Qt::ItemIsEditable; | 表示按位或运算
        }
        return flags;
    }

    bool setName(int id, QString name)
    {
        // todo
        //queryRefresh.prepare("update medical_monitor5.device set refresh=now() where dev_id = :dev_id");
        bool ok;
        QSqlQuery query;
        query.prepare("update medical_monitor5.patient set name= :name where id = :id");
        query.bindValue(":name", name);
        query.bindValue(":id", id);
        ok = query.exec();
        qDebug()<<id<<name;
        if (!ok)
            qDebug()<<"setname error:"<<query.lastError();
        return true;
    }

    bool setSex(int id, QString sex)
    {
        // todo
        //queryRefresh.prepare("update medical_monitor5.device set refresh=now() where dev_id = :dev_id");
        bool ok;
        QSqlQuery query;
        query.prepare("update medical_monitor5.patient set sex= :sex where id = :id");
        query.bindValue(":sex", sex);
        query.bindValue(":id", id);
        ok = query.exec();
        qDebug()<<id<<sex;
        if (!ok)
            qDebug()<<"setname error:"<<query.lastError();
        return true;
    }


    bool setData(const QModelIndex &index, const QVariant &value, int) override
    {
        if (index.column() <1 || index.column() > 2 )
        {
            return false;
        }

        // 获取当前行的第0列内容
        QModelIndex primaryKeyIndex = QSqlQueryModel::index(index.row(),0);

        int id = this->data(primaryKeyIndex).toInt();

        bool ok;
        if (index.column() == 1)
        {
            // 修改病人姓名
            ok = this->setName(id, value.toString());
        }else if(index.column() == 2)
        {
            // 修改病人性别
            ok = this->setSex(id, value.toString());
        }

        if(ok)
        {
//            this->setQuery("select * from patient");
            this->setQuery("select * from patient "
                          "left join device_patient on patient.id = device_patient.patient_id "
                          "left join device on device.dev_id = device_patient.dev_id");
        }

        return ok;
    }

};

int main (int argc, char *argv[])
{
    bool ok;

    QApplication a(argc, argv);
    // 创建view
    QTableView *view = new QTableView();
    // 创建模型
    QSqlQueryModel model;
    // 创建病人列表view
    QTableView *patientView = new QTableView();
    // 创建病人模型
    PatientModel patients;

    QTableView *patientDevView = new QTableView();
    // 创建病人模型
    PatientModel patientsDev;

    // 加载驱动
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

    // 配置服务器地址
    db.setHostName("localhost");

    // 数据库名字
    db.setDatabaseName("medical_monitor5");

    // 用户名密码
    db.setUserName("doctor5");
    db.setPassword("1");

    // 打开连接
    // 要判断连接是否成功
    bool dbOk = db.open();
    if(dbOk)
    {
        qDebug()<<"连接成功";
        QMessageBox::information(NULL, "打开连接", "连接成功");

    }else
    {
        qDebug()<<"连接失败";
        QMessageBox::critical(0 , "注意连接" , "连接失败", QMessageBox::Ok | QMessageBox::Default , QMessageBox::Cancel | QMessageBox::Escape , 	0 );
        qDebug()<<db.lastError();//显示错误类型
        return a.exec();
    }

    // 给doctor表添加1条记录

//    QSqlQuery query(db);
    if(dbOk)
    {
        bool queryOk;
        // QSqlQuery负载执行SQL语句
        QSqlQuery query;
        // 添加一行
//        queryOk = query.exec("insert into medical_monitor5.doctor (uid, name, mobile) values ('doctor2', '张无忌', 13987654322)");
//        if(!queryOk)
//        {
//            qDebug()<<query.lastError();
//        }
        // 数据查询，法1
        queryOk = query.exec("select * from medical_monitor5.doctor where uid = 'doctor2'");
        qDebug()<<query.size();
        if (query.size() > 0)
        {
            while(query.next())
            {
                qDebug()<<query.value("uid").toString()<<query.value("name").toString()<<query.value("mobile").toULongLong();
            }
        }

        // preapare方法查询，法2
        query.prepare("select * from medical_monitor5.doctor where uid = :id");
        QString uid = "doctor1";
        query.bindValue(":id", uid);
        queryOk = query.exec();
        if(!queryOk)
        {
            qDebug()<<query.lastError();
        }
        qDebug()<<query.size();
        if (query.size() > 0)
        {
            while(query.next())
            {
                qDebug()<<query.value("uid").toString()<<query.value("name").toString()<<query.value("mobile").toULongLong();
            }
        }



//模拟终端设备
        // 查询device表
        QSqlQuery queryDev;
        queryDev.prepare("select * from medical_monitor5.device where serial = :serial");
        queryDev.bindValue(":serial", "DEV-002");
        ok = queryDev.exec();
        int dev_id = 0;
        if(!ok)
        {
            qDebug()<<"设备ID查询错误"<<queryDev.lastError();
        }else
        {
            if (queryDev.size() > 0 && queryDev.next())
            {
                dev_id = queryDev.value("dev_id").toInt();
                qDebug()<<"dev_id = "<<dev_id;
            }else
            {
                // 增加当前设备
                ok = queryDev.exec("insert into medical_monitor5.device (serial) values ('DEV-002')");
                if(!ok)
                {
                    qDebug()<<"增加设备错误"<<queryDev.lastError();
                }else
                {
                    qDebug()<<"增加设备成功";
                }
            }
        }

        // 波形数据上传
        if( dev_id != 0)
        {
            QSqlQuery queryWave;
            bool queryWaveOk;
            //short samples[10] = {2040, 2041, 2042, 2043, 2044, 2045, 2046, 2047, 2048, 2049};
            static  const unsigned char  Spo2_DemoData[248] =
            {
                0x21, 0x21, 0x22, 0x22, 0x23, 0x22, 0x22, 0x21, 0x21,
                0x20, 0x20, 0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1b, 0x1a,
                0x19, 0x18, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12,
                0x11, 0x11, 0x10, 0x10, 0x0f, 0x0f, 0x0e, 0x0d, 0x0c,
                0x0c, 0x0b, 0x0a, 0x09, 0x09, 0x08, 0x07, 0x06, 0x06,
                0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x01,
                0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x01, 0x03, 0x06, 0x0a, 0x0e, 0x13, 0x18, 0x1d,
                0x23, 0x28, 0x2d, 0x32, 0x37, 0x3b, 0x40, 0x44, 0x49,
                0x4d, 0x51, 0x54, 0x57, 0x58, 0x5a, 0x5a, 0x5a, 0x59,
                0x59, 0x57, 0x55, 0x52, 0x50, 0x4c, 0x49, 0x45, 0x42,
                0x3e, 0x3b, 0x37, 0x34, 0x31, 0x2f, 0x2c, 0x29, 0x26,
                0x24, 0x22, 0x20, 0x1f, 0x1f, 0x1e, 0x1e, 0x1d, 0x1d,
                0x1e, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x22,
                0x22, 0x21, 0x21, 0x20, 0x20, 0x1f, 0x1e, 0x1d, 0x1c,
                0x1b, 0x1b, 0x1a, 0x19, 0x18, 0x18, 0x17, 0x16, 0x15,
                0x14, 0x13, 0x12, 0x11, 0x11, 0x10, 0x10, 0x0f, 0x0f,
                0x0e, 0x0d, 0x0c, 0x0c, 0x0b, 0x0a, 0x09, 0x09, 0x08,
                0x07, 0x06, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03,
                0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x06, 0x0a, 0x0e,
                0x13, 0x18, 0x1d, 0x23, 0x28, 0x2d, 0x32, 0x37, 0x3b,
                0x40, 0x44, 0x49, 0x4d, 0x51, 0x54, 0x57, 0x58, 0x5a,
                0x5a, 0x5a, 0x59, 0x59, 0x57, 0x55, 0x52, 0x50, 0x4c,
                0x49, 0x45, 0x42, 0x3e, 0x3b, 0x37, 0x34, 0x31, 0x2f,
                0x2c, 0x29, 0x26, 0x24, 0x22, 0x20, 0x1f, 0x1f, 0x1e,
                0x1e, 0x1d, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x21, 0x22,
                0x22, 0x23, 0x22, 0x22, 0x21,
            };
            QByteArray waves((char*)Spo2_DemoData, sizeof(Spo2_DemoData)); // QByteArray wave[10*2];
            queryWave.prepare("insert into medical_monitor5.ecg_sample (dev_id, value, time) values (:dev_id, :value, :time)");
            queryWave.bindValue(":dev_id", dev_id);//往数据库里面写东西
            queryWave.bindValue(":value", waves);
            queryWave.bindValue(":time", QDateTime::currentDateTime());
            queryWaveOk = queryWave.exec();
            if(!queryWaveOk)
                qDebug()<<"wave update: "<<queryWave.lastError();
        }else
        {
            qDebug()<<"wave update error no dev_id";
        }

        // 更新refresh
        if( dev_id != 0)
        {
            QSqlQuery queryRefresh;
            queryRefresh.prepare("update medical_monitor5.device set refresh=now() where dev_id = :dev_id");
            queryRefresh.bindValue(":dev_id", dev_id);
            ok = queryRefresh.exec();
            if(ok)
            {
                qDebug()<<"设备在线状态已更新";
            }else
            {
                qDebug()<<"设备在线更新失败"<<queryRefresh.lastError();
            }
        }


        // 获取数据
        model.setQuery("SELECT dev_id, serial, now()-refresh<20 AS online FROM device");
        // 配置表格显示控件的模型
        view->setModel(&model);
        view->show();

        // 显示病人列表
//        patients.setQuery("select * from patient");
//        patientView->setModel(&patients);
//        patientView->show();

        // 显示病人设备关联表
//        patientsDev.setQuery("select * "
//                          "from patient, device, device_patient "
//                          "where "
//                          "   patient.id = device_patient.patient_id"
//                          "   and device.dev_id = device_patient.dev_id");
        patientsDev.setQuery("select * from patient "
                          "left join device_patient on patient.id = device_patient.patient_id "
                          "left join device on device.dev_id = device_patient.dev_id");
        patientDevView->setModel(&patientsDev);
        patientDevView->show();

    } // dbOk = True

    return a.exec();
}
