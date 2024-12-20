CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
# CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
FIND_PACKAGE(Qt5Widgets REQUIRED)

SET(helloqt_SOURCES main.cpp helloqtwindow.cpp)
SET(helloqt_HEADERS helloqtwindow.h)
# SET(helloqt_UIS helloqtwindow.ui)
SET(helloqt_UIS mainwindow.ui)
find_package(OpenCV REQUIRED)
set(yaml-cpp_DIR "/usr/local/lib/cmake/yaml-cpp")
find_package(yaml-cpp REQUIRED)
QT5_WRAP_UI(helloqt_UIS_HEADERS ${helloqt_UIS})

find_package(Eigen3 REQUIRED)
set(Eigen3_LIBRARIES Eigen3::Eigen)
find_package(Python3 COMPONENTS Development NumPy REQUIRED)

add_executable(mvc 
	${helloqt_UIS_HEADERS}
	MVC/mcl_view.cpp
	MVC/mcl_view.hpp
	MVC/main.cpp
	MVC/mcl_model.cpp
	MVC/mcl_model.hpp
	MVC/mcl_controller.cpp
	MVC/mcl_controller.hpp
	MVC/types.hpp
	MVC/field.cpp
	MVC/field.hpp
	MVC/item.hpp
	MVC/item.cpp
	MVC/preprocessor.cpp
	MVC/preprocessor.hpp
)

target_link_libraries(mvc ${OpenCV_LIBS} ${YAML_CPP_LIBRARIES}
	Qt5::Gui
	Qt5::Core
	Qt5::Widgets
)
