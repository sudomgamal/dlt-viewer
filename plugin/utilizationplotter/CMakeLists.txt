# Copyright (C) 2024, Mohammed Gamal
#
# This file is part of COVESA DLT-Viewer project.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.
#
# List of changes:
# 16.Sept.2024, Mohammed Gamal <mg.moh.gamal@gmail.com>, Original Author
#
add_library(utilizationplotter MODULE
    utilizationplotter.cpp
    qcustomplot.cpp
    form.cpp)

add_definitions("-Wa,-mbig-obj")

target_link_libraries(utilizationplotter qdlt ${QT_PREFIX}::Widgets ${QT_PREFIX}::PrintSupport)
add_plugin(utilizationplotter)
