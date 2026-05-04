import XCTest
@testable import Rhubarb

final class RhubarbTests: XCTestCase {
    func testVersionIsNotEmpty() {
        XCTAssertFalse(Rhubarb.version.isEmpty)
    }

    func testBundledResourcesLoad() throws {
        try Rhubarb.useBundledResources()
    }
}
