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
set(QT_PREFIX "Qt6")

add_library(performancegraphs MODULE
    consumer.cpp  consumer.h  form.cpp  form.h  form.ui  metric.cpp  metric.h  modelitem.cpp  modelitem.h  qcustomplot.cpp  qcustomplot.h  usagegraph.cpp  usagegraph.h  utilizationplotter.cpp  utilizationplotter.hpp
)

# add_definitions("-Wa,-mbig-obj")

target_link_libraries(performancegraphs qdlt ${QT_PREFIX}::Widgets ${QT_PREFIX}::PrintSupport)
add_plugin(performancegraphs)
