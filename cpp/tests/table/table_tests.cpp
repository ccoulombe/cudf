/*
 * Copyright (c) 2019, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include <cudf/column/column.hpp>
//#include <cudf/null_mask.hpp>
//#include <cudf/types.hpp>
//#include <cudf/utilities/type_dispatcher.hpp>
//#include <tests/utilities/base_fixture.hpp>
//#include <tests/utilities/column_utilities.hpp>
//#include <tests/utilities/cudf_gtest.hpp>
//#include <tests/utilities/type_list_utilities.hpp>
//#include <tests/utilities/type_lists.hpp>

#include <cudf/column/column.hpp>
#include <cudf/column/column_view.hpp>
#include <cudf/table/table.hpp>
#include <cudf/table/table_view.hpp>

#include <tests/utilities/base_fixture.hpp>
#include <tests/utilities/column_utilities.hpp>
#include <tests/utilities/column_wrapper.hpp>

#include <random>
#include <memory>

template <typename T>
using column_wrapper = cudf::test::fixed_width_column_wrapper<T>;

using CVector     = std::vector<std::unique_ptr<cudf::column>>;
using column      = cudf::column;
using column_view = cudf::column_view;
using TView       = cudf::table_view;
using Table       = cudf::experimental::table;

struct TableTest : public cudf::test::BaseFixture {};

TEST_F(TableTest, EmptyColumnedTable)
{
    std::vector<column_view> cols{};

    TView input(cols);
    cudf::size_type expected = 0;

    EXPECT_EQ(input.num_columns(), expected);
}

TEST_F(TableTest, GetTableWithSelectedColumns)
{
  column_wrapper <int8_t>  col1{{1,2,3,4}};
  column_wrapper <int16_t> col2{{1,2,3,4}};
  column_wrapper <int32_t> col3{{4,5,6,7}};
  column_wrapper <int64_t> col4{{4,5,6,7}};

  CVector cols;
  cols.push_back(col1.release());
  cols.push_back(col2.release());
  cols.push_back(col3.release());
  cols.push_back(col4.release());

  Table t(std::move(cols));

  cudf::table_view selected_tview = t.select(std::vector<cudf::size_type>{2,3});
  cudf::test::expect_columns_equal(t.view().column(2), selected_tview.column(0));
  cudf::test::expect_columns_equal(t.view().column(3), selected_tview.column(1));
}

TEST_F(TableTest, SelectingOutOfBounds)
{
  column_wrapper <int8_t > col1{{1,2,3,4}};
  column_wrapper <int16_t> col2{{1,2,3,4}};

  CVector cols;
  cols.push_back(col1.release());
  cols.push_back(col2.release());

  Table t(std::move(cols));

  EXPECT_THROW (t.select(std::vector<cudf::size_type>{0,1,2}), std::out_of_range);
}

TEST_F(TableTest, SelectingNoColumns)
{
  column_wrapper <int8_t > col1{{1,2,3,4}};
  column_wrapper <int16_t> col2{{1,2,3,4}};

  CVector cols;
  cols.push_back(col1.release());
  cols.push_back(col2.release());
  Table t(std::move(cols));
  TView selected_table = t.select(std::vector<cudf::size_type>{});

  EXPECT_EQ(selected_table.num_columns(), 0);
}

TEST_F(TableTest, ConcatTables)
{
  column_wrapper <int8_t > col1{{1,2,3,4}};
  column_wrapper <int16_t> col2{{1,2,3,4}};

  std::vector<TView> views;
  views.emplace_back(std::vector<column_view>{col1});
  views.emplace_back(std::vector<column_view>{col2});
  auto concat_view = cudf::experimental::concat(views);
  cudf::test::expect_columns_equal(concat_view.column(0), views[0].column(0));
  cudf::test::expect_columns_equal(concat_view.column(1), views[1].column(0));
}

TEST_F(TableTest, ConcatTablesRowsMismatch)
{
  column_wrapper <int8_t > col1{{1,2,3,4}};
  column_wrapper <int16_t> col2{{1,2,3}};

  std::vector<TView> views;
  views.emplace_back(std::vector<column_view>{col1});
  views.emplace_back(std::vector<column_view>{col2});
  EXPECT_THROW (cudf::experimental::concat(views), cudf::logic_error);
}

TEST_F(TableTest, ConcatEmptyTables)
{
  std::vector<TView> views;
  views.emplace_back(std::vector<column_view>{});
  views.emplace_back(std::vector<column_view>{});
  auto concat_view = cudf::experimental::concat(views);
  EXPECT_EQ(concat_view.num_columns(), 0);
}
