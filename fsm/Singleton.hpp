#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <mutex>
#include <memory>

namespace nd
{
    //---------------------------------------------------------------------------
    //DataType normal
    template<typename DataType, int instanceId = 0>
    class Singleton 
    {
    public: 
        static DataType* instance()
        {
            if (NULL == dataHolderM.get())
            {
                std::lock_guard<std::mutex> lock(dbLockMutexM);
                if (NULL == dataHolderM.get())
                {
                    dataHolderM.reset(new DataType);
                }
            }
            return dataHolderM.get();

        }
    private:
        Singleton(){};
        ~Singleton(){};

        static std::shared_ptr<DataType> dataHolderM;
        static std::mutex dbLockMutexM;
    };

    template<typename DataType, int instanceId>
    std::shared_ptr<DataType> Singleton<DataType, instanceId>::dataHolderM;

    template<typename DataType, int instanceId>
    std::mutex Singleton<DataType, instanceId>::dbLockMutexM;
	
    //---------------------------------------------------------------------------
    //DataType with param
    template<typename DataType, int instanceId = 0>
    class ParamSingleton 
    {
    public: 
        static DataType* instance()
        {
            if (NULL == dataHolderM.get())
            {
                std::lock_guard<std::mutex> lock(dbLockMutexM);
                if (NULL == dataHolderM.get())
                {
                    dataHolderM.reset(new DataType(instanceId));
                }
            }
            return dataHolderM.get();

        }
    private:
        ParamSingleton(){};
        ~ParamSingleton(){};

        static std::shared_ptr<DataType> dataHolderM;
        static std::mutex dbLockMutexM;
    };

    template<typename DataType, int instanceId>
    std::shared_ptr<DataType> ParamSingleton<DataType, instanceId>::dataHolderM;

    template<typename DataType, int instanceId>
    std::mutex ParamSingleton<DataType, instanceId>::dbLockMutexM;
	
    //---------------------------------------------------------------------------
	//DataType with init
    template<typename DataType, int instanceId = 0>
    class InitDataSingleton 
    {
    public: 
        static DataType* instance()
        {
            if (NULL == dataHolderM.get())
            {
                std::lock_guard<std::mutex> lock(dbLockMutexM);
                if (NULL == dataHolderM.get())
                {
					DataType* data = new DataType;
					data->init();
                    dataHolderM.reset(data);
                }
            }
            return dataHolderM.get();

        }
    private:
        InitDataSingleton(){};
        ~InitDataSingleton(){};

        static std::shared_ptr<DataType> dataHolderM;
        static std::mutex dbLockMutexM;
    };

    template<typename DataType, int instanceId>
    std::shared_ptr<DataType> InitDataSingleton<DataType, instanceId>::dataHolderM;

    template<typename DataType, int instanceId>
    std::mutex InitDataSingleton<DataType, instanceId>::dbLockMutexM;

    //---------------------------------------------------------------------------
	//DataType with init param
    template<typename DataType, int instanceId = 0>
    class InitParamSingleton 
    {
    public: 
        static DataType* instance()
        {
            if (NULL == dataHolderM.get())
            {
                std::lock_guard<std::mutex> lock(dbLockMutexM);
                if (NULL == dataHolderM.get())
                {
					DataType* data = new DataType;
					data->init(instanceId);
                    dataHolderM.reset(data);
                }
            }
            return dataHolderM.get();

        }
    private:
        InitParamSingleton(){};
        ~InitParamSingleton(){};

        static std::shared_ptr<DataType> dataHolderM;
        static std::mutex dbLockMutexM;
    };

    template<typename DataType, int instanceId>
    std::shared_ptr<DataType> InitParamSingleton<DataType, instanceId>::dataHolderM;

    template<typename DataType, int instanceId>
    std::mutex InitParamSingleton<DataType, instanceId>::dbLockMutexM;
}

#endif /* SINGLETON_HPP */

