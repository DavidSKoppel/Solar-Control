cmake_minimum_required(VERSION 3.10) # setting this is required
project(allmyprojects)      # this sets the project name

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    c_questdb_client_proj
    GIT_REPOSITORY https://github.com/questdb/c-questdb-client.git
    GIT_TAG 5.1.0)

FetchContent_MakeAvailable(c_questdb_client_proj)

if(NOT TARGET spdlog)
  find_package(spdlog REQUIRED)
endif()

file(GLOB_RECURSE asym src/sodiumAsymmetric.cpp )
add_executable(Asymmetric ${asym})
target_compile_options(Asymmetric PUBLIC -Wall -Wfloat-conversion)
target_include_directories(Asymmetric PUBLIC src )
target_link_libraries(Asymmetric sodium)

#file(GLOB_RECURSE mqttHashCheck src/mqttHashingCheck.cpp )
#add_executable(mqttHashingServer ${mqttHashCheck})
#target_compile_options(mqttHashingServer PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(mqttHashingServer PUBLIC src)
#target_link_libraries(mqttHashingServer paho-mqttpp3 paho-mqtt3as sodium)

#file(GLOB_RECURSE mqttHashSend src/mqttHashingSend.cpp )
#add_executable(mqttHashingClient ${mqttHashSend})
#target_compile_options(mqttHashingClient PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(mqttHashingClient PUBLIC src)
#target_link_libraries(mqttHashingClient paho-mqttpp3 paho-mqtt3as sodium)

#file(GLOB_RECURSE crypt src/sodiumEncryption.cpp )
#add_executable(Encryption ${crypt})
#target_compile_options(Encryption PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(Encryption PUBLIC src )
#target_link_libraries(Encryption sodium) 

#file(GLOB_RECURSE crypt src/sodiumHashPass.cpp )
#add_executable(sodiumHashPass ${crypt})
#target_compile_options(sodiumHashPass PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(sodiumHashPass PUBLIC src )
#target_link_libraries(sodiumHashPass sodium) 

file(GLOB_RECURSE questDB  src/questDBInsert.cpp)
add_executable(questDatabaseInsert ${questDB})
target_compile_options(questDatabaseInsert PUBLIC -Wall -Wfloat-conversion)
target_include_directories(questDatabaseInsert PUBLIC src )
target_link_libraries(questDatabaseInsert questdb_client)

#file(GLOB_RECURSE marmqtt  src/mariadbMQTTInsert.cpp)
#add_executable(mariadbmqtt ${marmqtt})
#target_compile_options(mariadbmqtt PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(mariadbmqtt PUBLIC src )
#target_link_libraries(mariadbmqtt paho-mqttpp3 paho-mqtt3as mariadbcpp spdlog::spdlog)

#file(GLOB_RECURSE msrc  src/mariadbInsert.cpp)
#add_executable(mariadbinsert ${msrc})
#target_compile_options(mariadbinsert PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(mariadbinsert PUBLIC src )
#target_link_libraries(mariadbinsert mariadbcpp spdlog::spdlog)


#file(GLOB_RECURSE mqttpub src/mqttPub.cpp )
#add_executable(mqttpublisher ${mqttpub})
#target_compile_options(mqttpublisher PUBLIC -Wall -Wfloat-conversion)
#target_include_directories(mqttpublisher PUBLIC src)
#target_link_libraries(mqttpublisher paho-mqttpp3 paho-mqtt3as)


file(GLOB_RECURSE mqttsub src/mqttSub.cpp )
add_executable(mqttsubscriber ${mqttsub})
target_compile_options(mqttsubscriber PUBLIC -Wall -Wfloat-conversion)
target_include_directories(mqttsubscriber PUBLIC src)
target_link_libraries(mqttsubscriber paho-mqttpp3 paho-mqtt3as)
