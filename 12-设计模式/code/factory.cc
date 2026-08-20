#include <iostream>
#include <string>

// 导出接口类
class IExport {
public:
    virtual bool Export(const std::string &data) = 0;
    virtual ~IExport() {}
};

// XML 导出类
class ExportXml : public IExport {
public:
    virtual bool Export(const std::string &data) override {
        std::cout << "Exporting XML: " << data << std::endl;
        return true;
    }
};

// JSON 导出类
class ExportJson : public IExport {
public:
    virtual bool Export(const std::string &data) override {
        std::cout << "Exporting JSON: " << data << std::endl;
        return true;
    }
};

// 文本导出类
class ExportTxt : public IExport {
public:
    virtual bool Export(const std::string &data) override {
        std::cout << "Exporting Text: " << data << std::endl;
        return true;
    }
};

// CSV 导出类
class ExportCSV : public IExport {
public:
    virtual bool Export(const std::string &data) override {
        std::cout << "Exporting CSV: " << data << std::endl;
        return true;
    }
};

// 导出工厂接口类
class IExportFactory {
public:
    IExportFactory() : _export(nullptr) {}
    virtual ~IExportFactory() {
        if (_export) {
            delete _export;
            _export = nullptr;
        }
    }
    bool Export(const std::string &data) {
        if (_export == nullptr) {
            _export = NewExport();
        }
        return _export->Export(data);
    }

protected:
    virtual IExport *NewExport() = 0;

private:
    IExport* _export;
};

// XML 导出工厂类
class ExportXmlFactory : public IExportFactory {
protected:
    virtual IExport *NewExport() override {
        return new ExportXml();
    }
};

// JSON 导出工厂类
class ExportJsonFactory : public IExportFactory {
protected:
    virtual IExport *NewExport() override {
        return new ExportJson();
    }
};

// 文本导出工厂类
class ExportTxtFactory : public IExportFactory {
protected:
    virtual IExport *NewExport() override {
        return new ExportTxt();
    }
};

// CSV 导出工厂类
class ExportCSVFactory : public IExportFactory {
protected:
    virtual IExport *NewExport() override {
        IExport *temp = new ExportCSV();
        return temp;
    }
};

int main() {
    // 使用文本导出工厂
    IExportFactory *factory = new ExportTxtFactory();
    factory->Export("hello world");
    delete factory;

    // 可以再测试其他工厂，比如 XML 导出工厂
    IExportFactory *xmlFactory = new ExportXmlFactory();
    xmlFactory->Export("XML data");
    delete xmlFactory;

    return 0;
}
