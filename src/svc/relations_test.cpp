#include <grpcxx/request.h>
#include <gtest/gtest.h>

#include "db/testing.h"

#include "common.h"
#include "svc.h"

using namespace sentium::api::v1::Relations;

class svc_RelationsTest : public testing::Test {
protected:
	static void SetUpTestSuite() {
		db::testing::setup();

		// Clear data
		db::pg::exec("truncate table principals cascade;");
		db::pg::exec("truncate table tuples;");
	}

	static void TearDownTestSuite() { db::testing::teardown(); }
};

TEST_F(svc_RelationsTest, Create) {
	grpcxx::context ctx;
	svc::Relations  svc;

	// Success: create relation
	{
		rpcCreate::request_type request;

		auto *left = request.mutable_left_entity();
		left->set_id("left");
		left->set_type("svc_RelationsTest.Create");

		request.set_relation("relation");

		auto *right = request.mutable_right_entity();
		right->set_id("right");
		right->set_type("svc_RelationsTest.Create");

		rpcCreate::result_type result;
		EXPECT_NO_THROW(result = svc.call<rpcCreate>(ctx, request));

		EXPECT_EQ(grpcxx::status::code_t::ok, result.status.code());
		ASSERT_TRUE(result.response);
		EXPECT_EQ(1, result.response->cost());

		auto &actual = result.response->tuple();
		EXPECT_FALSE(actual.id().empty());
		EXPECT_EQ("", actual.space_id());

		EXPECT_FALSE(actual.has_left_principal_id());
		EXPECT_EQ(left->id(), actual.left_entity().id());
		EXPECT_EQ(left->type(), actual.left_entity().type());
		EXPECT_EQ(request.relation(), actual.relation());
		EXPECT_FALSE(actual.has_right_principal_id());
		EXPECT_EQ(right->id(), actual.right_entity().id());
		EXPECT_EQ(right->type(), actual.right_entity().type());

		EXPECT_FALSE(actual.has_strand());
		EXPECT_FALSE(actual.has_attrs());
		EXPECT_FALSE(actual.has_ref_id());
	}

	// Success: create relation with principals
	{
		db::Principal principal({.id = "svc_RelationsTest.Create-with_principals"});
		ASSERT_NO_THROW(principal.store());

		rpcCreate::request_type request;
		request.set_left_principal_id(principal.id());
		request.set_relation("relation");
		request.set_right_principal_id(principal.id());

		rpcCreate::result_type result;
		EXPECT_NO_THROW(result = svc.call<rpcCreate>(ctx, request));

		EXPECT_EQ(grpcxx::status::code_t::ok, result.status.code());
		ASSERT_TRUE(result.response);
		EXPECT_EQ(1, result.response->cost());

		auto &actual = result.response->tuple();
		EXPECT_FALSE(actual.id().empty());
		EXPECT_EQ("", actual.space_id());

		EXPECT_FALSE(actual.has_left_entity());
		EXPECT_EQ(principal.id(), actual.left_principal_id());
		EXPECT_EQ(request.relation(), actual.relation());
		EXPECT_FALSE(actual.has_right_entity());
		EXPECT_EQ(principal.id(), actual.right_principal_id());

		EXPECT_FALSE(actual.has_strand());
		EXPECT_FALSE(actual.has_attrs());
		EXPECT_FALSE(actual.has_ref_id());
	}

	// Success: create relation with space-id
	{
		grpcxx::detail::request r(1);
		r.header(
			std::string(svc::common::space_id_v),
			"space_id:svc_RelationsTest.Create-with_space_id");

		grpcxx::context ctx(r);

		rpcCreate::request_type request;

		auto *left = request.mutable_left_entity();
		left->set_id("left");
		left->set_type("svc_RelationsTest.Create-with_space_id");

		request.set_relation("relation");

		auto *right = request.mutable_right_entity();
		right->set_id("right");
		right->set_type("svc_RelationsTest.Create-with_space_id");

		rpcCreate::result_type result;
		EXPECT_NO_THROW(result = svc.call<rpcCreate>(ctx, request));

		EXPECT_EQ(grpcxx::status::code_t::ok, result.status.code());
		ASSERT_TRUE(result.response);
		EXPECT_EQ(1, result.response->cost());

		auto &actual = result.response->tuple();
		EXPECT_FALSE(actual.id().empty());
		EXPECT_EQ("space_id:svc_RelationsTest.Create-with_space_id", actual.space_id());

		EXPECT_FALSE(actual.has_left_principal_id());
		EXPECT_EQ(left->id(), actual.left_entity().id());
		EXPECT_EQ(left->type(), actual.left_entity().type());
		EXPECT_EQ(request.relation(), actual.relation());
		EXPECT_FALSE(actual.has_right_principal_id());
		EXPECT_EQ(right->id(), actual.right_entity().id());
		EXPECT_EQ(right->type(), actual.right_entity().type());

		EXPECT_FALSE(actual.has_strand());
		EXPECT_FALSE(actual.has_attrs());
		EXPECT_FALSE(actual.has_ref_id());
	}

	// Error: invalid entity
	{
		rpcCreate::request_type request;

		auto *left = request.mutable_left_entity();
		left->set_id("left");
		left->set_type("svc_RelationsTest.Create-invalid_entity");

		request.set_relation("relation");

		auto *right = request.mutable_right_entity(); // empty id
		right->set_type("svc_RelationsTest.Create-invalid_entity");

		rpcCreate::result_type result;
		EXPECT_NO_THROW(result = svc.call<rpcCreate>(ctx, request));

		EXPECT_EQ(grpcxx::status::code_t::invalid_argument, result.status.code());
		ASSERT_FALSE(result.response);
	}

	// Error: invalid principal
	{
		db::Principal principal({.spaceId = "svc_RelationsTest.Create--invalid-principal"});
		ASSERT_NO_THROW(principal.store());

		rpcCreate::request_type request;
		request.set_left_principal_id(principal.id()); // space-id mismatch
		request.set_relation("relation");

		auto *right = request.mutable_right_entity();
		right->set_id("right");
		right->set_type("svc_RelationsTest.Create-invalid-principal");

		rpcCreate::result_type result;
		EXPECT_NO_THROW(result = svc.call<rpcCreate>(ctx, request));

		EXPECT_EQ(grpcxx::status::code_t::invalid_argument, result.status.code());
		ASSERT_FALSE(result.response);
	}
}