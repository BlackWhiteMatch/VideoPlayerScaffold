#pragma once
#include <fmt/core.h>
#include <iostream>
#include <memory>

#include <odb/database.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/mysql/database.hxx>
#include <odb/mysql/connection-factory.hxx>
#include <odb/transaction.hxx>
#include <type_traits>

namespace OdbUtil{
    namespace MySQL{
        struct MySQLSetting{
            std::string host = "127.0.0.1";
            std::string user = "root";
            std::string passwd = "123456";
            std::string db = "test_db";
            unsigned int port = 3306;
            std::string charset = "utf8mb4";
            unsigned int pool_size = 5;
        };

        inline std::shared_ptr<odb::database> createDatabase(const MySQLSetting& setting){
            std::unique_ptr<odb::mysql::connection_factory> pool(
                new odb::mysql::connection_pool_factory(setting.pool_size)
            );

            return std::make_shared<odb::mysql::database>(
                setting.user,
                setting.passwd,
                setting.db,
                setting.host,
                setting.port,
                nullptr,
                setting.charset,
                0,
                std::move(pool)
            );
        }

        template<typename Func>
        auto executeTransaction(const std::shared_ptr<odb::database>& db, Func&& func){
            odb::transaction t(db->begin());

            using R = std::invoke_result_t<Func>;
            if constexpr (std::is_void_v<R>){
                func();
                t.commit();
            }else{
                auto result = func();
                t.commit();
                return result;
            }
        }

        class Client{
        public:
            explicit Client(const MySQLSetting& setting)
                :_db(createDatabase(setting))
            {}

            template<typename Func>
            auto execute(Func&& func){
                return executeTransaction(_db, std::forward<Func>(func));
            }

            std::shared_ptr<odb::database> getDatabase() const { return _db; }

        private:
            std::shared_ptr<odb::database> _db;
        };
    }
}
