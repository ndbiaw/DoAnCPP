#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "crow_all.h"
#include "json.hpp"

using json = nlohmann::json;
std::string url_decode(const std::string& str) {
    std::string decoded;
    size_t len = str.length();
    for (size_t i = 0; i < len; ++i) {
        if (str[i] == '+') {
            decoded += ' ';
        }
        else if (str[i] == '%' && i + 2 < len) {
            int value = 0;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> value) {
                decoded += static_cast<char>(value);
                i += 2;
            }
            else {
                throw std::invalid_argument("Invalid URL-encoded string");
            }
        }
        else {
            decoded += str[i];
        }
    }
    return decoded;
}
int main()
{

    crow::SimpleApp app;

    std::ifstream file1("sinhvien.json");
    json j1;
    file1 >> j1;
    file1.close();

    std::ifstream file2("sach.json");
    json j2;
    file2 >> j2;
    file2.close();

    CROW_ROUTE(app, "/")([]() {
        auto page = crow::mustache::load("login.html");
        return page.render();
        });

    CROW_ROUTE(app, "/quanly/<string>/<string>/sach")
        ([](const crow::request& req, std::string idquanly, std::string maquanly) {
        std::string html = "";
        std::string status = "";
        json quanly;
        std::ifstream quanly_file("quanly.json");
        quanly_file >> quanly;
        for (auto& obj : quanly) {
            if (obj["IDQuanLy"] == idquanly && obj["MaQuanLy"] == maquanly) {
                status = "1";
                    break;
            }
        }
        if (status != "") {
            std::ifstream quanly_sach("sach.json");
            json sachql;
            quanly_sach >> sachql;
            quanly_sach.close();
            auto page = crow::mustache::load("quanly.html");
            for (const auto& item : sachql) {
                html += "<tr><td>" + std::to_string(item["IDsach"].get<int>()) + "</td><td>" + item["TenSach"].get<std::string>() + "</td><td id='" + std::to_string(item["IDsach"].get<int>()) + "'>" + std::to_string(item["SoLuong"].get<int>()) + "</td><td><button type=\"button\" onclick=\"fetch('./sach/del/" + std::to_string(item["IDsach"].get<int>()) + "').then(response => response.text()).then(data => {const td = document.getElementById('" + std::to_string(item["IDsach"].get<int>()) + "');if (td && td.tagName === 'TD') {const value = parseInt(td.textContent) - 1;td.textContent = value.toString();}}).catch(error => console.error(error));\">Xóa 1</button><button type=\"button\" onclick=\"fetch('./sach/add/" + std::to_string(item["IDsach"].get<int>()) + "').then(response => response.text()).then(data => {const td = document.getElementById('" + std::to_string(item["IDsach"].get<int>()) + "');if (td && td.tagName === 'TD') {const value = parseInt(td.textContent) + 1;td.textContent = value.toString();}}).catch(error => console.error(error));\">Thêm 1</button></td></tr>";
            }
            crow::mustache::context ctx({ {"addbut", "<button type=\"button\" onclick=\"var tenSach = prompt(\'Nhập tên sách:\');var soLuong = prompt(\'Nhập số lượng:\');fetch(`./themsach/${tenSach}/${soLuong}`).then(response => {if (!response.ok) {throw new Error('Network response was not ok');}return response.text();}).then(data => {console.log(data);location.reload();}).catch(error => {console.error('There was a problem with the fetch operation:', error);});\">Thêm Sách</button>"}, {"active1", "active"}, {"col1", "ID Sách"}, {"col2", "Tên Sách"}, {"col3", "Số Lượng"}, {"col4", "Thao Tác"}, {"contentql", html} });
            return page.render(ctx);
        }
        else {
            auto page = crow::mustache::load("error.html");
            return page.render();
        }
            });

    CROW_ROUTE(app, "/quanly/<string>/<string>/sach/<string>/<int>")
        ([&](const crow::request& req, std::string quanlyID, std::string quanlypass,
            std::string thaotac, int idsach) {
                std::ifstream sach_file("sach.json");
                json sach_data;
                sach_file >> sach_data;

                std::ifstream quanly_file("quanly.json");
                json quanly_data;
                quanly_file >> quanly_data;
                bool is_valid_quanly = false;
                for (const auto& quanly : quanly_data) {
                    if (quanly["IDQuanLy"] == quanlyID && quanly["MaQuanLy"] == quanlypass) {
                        is_valid_quanly = true;
                        break;
                    }
                }

                if (!is_valid_quanly) {
                    crow::response res(401);
                    res.end();
                    return res;
                }

                bool book_found = false;
                for (auto& book : sach_data) {
                    if (book["IDsach"] == idsach) {
                        book_found = true;

                        if (thaotac == "add") {
                            book["SoLuong"] = book["SoLuong"].get<int>() + 1;
                        }
                        else if (thaotac == "del") {
                            int quantity = book["SoLuong"].get<int>();
                            if (quantity > 0) {
                                book["SoLuong"] = quantity - 1;
                            }
                        }

                        std::ofstream output_file("sach.json");
                        output_file << sach_data.dump(4);
                        output_file.close();

                        crow::response res;
                        res.set_header("Content-Type", "text/html; charset=UTF-8");
                        res.body = "True";
                        return res;
                    }
                }

                if (!book_found) {
                    crow::response res(404);
                    res.end();
                    return res;
                }
                crow::response res(500);
                res.end();
                return res;
            });
    CROW_ROUTE(app, "/quanly/<string>/<string>/xoasv/<string>")
        ([&](const crow::request& req, std::string id_quanly, std::string ma_quanly, std::string ma_sv)
            {
                json sach;
                std::ifstream sach_file("sinhvien.json");
                sach_file >> sach;

                json quanly;
                std::ifstream quanly_file("quanly.json");
                quanly_file >> quanly;
                bool quanly_match = false;
                for (auto& q : quanly)
                {
                    if (q["IDQuanLy"] == id_quanly && q["MaQuanLy"] == ma_quanly)
                    {
                        quanly_match = true;
                        break;
                    }
                }

                if (!quanly_match)
                {
                    return crow::response(401, "Unauthorized");
                }

                bool student_deleted = false;
                json sach_new;
                for (auto& s : sach)
                {
                    if (s["MaSV"] == ma_sv)
                    {
                        student_deleted = true;
                    }
                    else
                    {
                        sach_new.push_back(s);
                    }
                }

                if (!student_deleted)
                {
                    return crow::response(404, "Student not found");
                }

                std::ofstream sach_out("sinhvien.json");
                sach_out << sach_new.dump(4);

                crow::response res;
                res.set_header("Content-Type", "text/html; charset=UTF-8");
                res.body = "True";
                return res;
            });

    CROW_ROUTE(app, "/quanly/<string>/<string>/xoasachmuon/<string>")
        ([&](const crow::request& req, std::string id_quanly, std::string ma_quanly, std::string ma_sv)
            {
                json sach;
                std::ifstream sach_file("sachmuon.json");
                sach_file >> sach;

                json quanly;
                std::ifstream quanly_file("quanly.json");
                quanly_file >> quanly;
                bool quanly_match = false;
                for (auto& q : quanly)
                {
                    if (q["IDQuanLy"] == id_quanly && q["MaQuanLy"] == ma_quanly)
                    {
                        quanly_match = true;
                        break;
                    }
                }

                if (!quanly_match)
                {
                    return crow::response(401, "Unauthorized");
                }

                bool student_deleted = false;
                json sach_new;
                for (auto& s : sach)
                {
                    if (s["MaSVMuon"] == ma_sv)
                    {
                        student_deleted = true;
                    }
                    else
                    {
                        sach_new.push_back(s);
                    }
                }

                if (!student_deleted)
                {
                    return crow::response(404, "Student not found");
                }

                std::ofstream sach_out("sachmuon.json");
                sach_out << sach_new.dump(4);

                crow::response res;
                res.set_header("Content-Type", "text/html; charset=UTF-8");
                res.body = "True";
                return res;
            });

    CROW_ROUTE(app, "/quanly/<string>/<string>/sinhvien")
        ([](const crow::request& req, std::string idquanly, std::string maquanly) {
        std::string html = "";
        std::string status = "";
        json quanly;
        std::ifstream quanly_file("quanly.json");
        quanly_file >> quanly;
        for (auto& obj : quanly) {
            if (obj["IDQuanLy"] == idquanly && obj["MaQuanLy"] == maquanly) {
                status = "1";
                break;
            }
        }
        if (status != "") {
            std::ifstream quanly_sv("sinhvien.json");
            json sinhvienql;
            quanly_sv >> sinhvienql;
            quanly_sv.close();
            auto page = crow::mustache::load("quanly.html");
            for (const auto& item : sinhvienql) {
                html += "<tr><td>" + item["MaSV"].get<std::string>() + "</td><td>" + item["CCCD"].get<std::string>() + "</td><td>" + item["Ten"].get<std::string>() + "</td><td><button type=\"button\" onclick=\"fetch('./xoasv/" + item["MaSV"].get<std::string>() + "').then(response => {if (response.ok) {location.reload();} else {throw new Error('Xóa sinh viên không thành công.');}}).catch(error => console.error(error));\">Xóa Sinh Viên</button></td></tr>";
            }
            crow::mustache::context ctx({ {"addbut", "<button type=\"button\" onclick=\"var CCCC = prompt(\'Nhập số căn cước:\');var MASV = prompt(\'Nhập Mã sinh viên:\');var TEN = prompt(\'Nhập Tên sinh viên:\');fetch(`./themsv/${CCCC}/${MASV}/${TEN}`).then(response => {if (!response.ok) {throw new Error('Network response was not ok');}return response.text();}).then(data => {console.log(data);location.reload();}).catch(error => {console.error('There was a problem with the fetch operation:', error);});\">Thêm Sinh Viên</button>"}, {"active2", "active"}, {"col1", "Mã Sinh Viên"}, {"col2", "Số Căn Cước"}, {"col3", "Họ và Tên"}, {"col4", "Thao Tác"}, {"contentql", html} });
            return page.render(ctx);
        }
        else {
            auto page = crow::mustache::load("error.html");
            return page.render();
        }
            });

    CROW_ROUTE(app, "/quanly/<string>/<string>/muonsach")
        ([](const crow::request& req, std::string idquanly, std::string maquanly) {
        std::string html = "";
        std::string status = "";
        json quanly;
        std::ifstream quanly_file("quanly.json");
        quanly_file >> quanly;
        for (auto& obj : quanly) {
            if (obj["IDQuanLy"] == idquanly && obj["MaQuanLy"] == maquanly) {
                status = "1";
                break;
            }
        }
        if (status != "") {
            std::ifstream quanly_muonsach("sachmuon.json");
            json muonsachql;
            quanly_muonsach >> muonsachql;
            quanly_muonsach.close();
            auto page = crow::mustache::load("quanly.html");
            for (const auto& item : muonsachql) {
                html += "<tr><td>" + item["IDvaSach"].get<std::string>() + "</td><td>" + item["MaSVMuon"].get<std::string>() + "</td><td>" + item["TenSVMuon"].get<std::string>() + "</td><td><button type=\"button\" onclick=\"fetch('./xoasachmuon/" + item["MaSVMuon"].get<std::string>() + "').then(response => {if (response.ok) {location.reload();} else {throw new Error('Xóa sách không thành công.');}}).catch(error => console.error(error));\">Xóa Toàn Bộ Sách Sinh Viên Đang Mượn</button></td></tr>";
            }
            crow::mustache::context ctx({ {"active3", "active"}, {"col1", "ID + Tên Sách"}, {"col2", "Mã Sinh Viên"}, {"col3", "Họ và Tên"}, {"col4", "Thao Tác"}, {"contentql", html} });
            return page.render(ctx);
        }
        else {
            auto page = crow::mustache::load("error.html");
            return page.render();
        }
            });

    CROW_ROUTE(app, "/quanly/<string>/<string>/quantrivien")
        ([](const crow::request& req, std::string idquanly, std::string maquanly) {
        std::string html = "";
        std::string status = "";
        json quanly;
        std::ifstream quanly_file("quanly.json");
        quanly_file >> quanly;
        for (auto& obj : quanly) {
            if (obj["IDQuanLy"] == idquanly && obj["MaQuanLy"] == maquanly) {
                status = "1";
                break;
            }
        }
        if (status != "") {
            std::ifstream quanly_admin("quanly.json");
            json adminql;
            quanly_admin >> adminql;
            quanly_admin.close();
            auto page = crow::mustache::load("quanly.html");
            for (const auto& item : adminql) {
                html += "<tr><td>" + item["IDQuanLy"].get<std::string>() + "</td><td>" + item["MaQuanLy"].get<std::string>() + "</td><td>Cao Nhất</td><td></td></tr>";
            }
            crow::mustache::context ctx({ {"active4", "active"}, {"col1", "Tên Tài Khoản"}, {"col2", "Mật Khẩu"}, {"col3", "Đặc Quyền"}, {"col4", "<button type=\"button\" onclick=\"window.location.href = '/thuvien/QuanTriVienLogin/QuanTriVien';\">Đến Giao Diện Mượn Sách</button>"}, {"contentql", html} });
            return page.render(ctx);
        }
        else {
            auto page = crow::mustache::load("error.html");
            return page.render();
        }
            });

    CROW_ROUTE(app, "/thuvien/<string>/<string>")
        ([j1, j2](const crow::request& req, std::string masv, std::string cccd) {
        std::string html = "";
        std::string ten = "";
        for (auto& obj : j1) {
            std::string html = "";
            if (obj["MaSV"] == masv && obj["CCCD"] == cccd) {
                ten = obj["Ten"];
                break;
            }
        }

        if (ten != "") {
            std::ifstream file2("sach.json");
            json j2;
            file2 >> j2;
            file2.close();
            auto page = crow::mustache::load("thuvien.html");
            for (const auto& item : j2) {
                html += "<div class='col mb-5'><div class='card h-100'><div class='badge bg-dark text-white position-absolute' style='top:.5rem;right:.5rem'>ID: " + std::to_string(item["IDsach"].get<int>()) + "</div><div class='card-body p-4'><div class='text-center'><h5 class='fw-bolder'>" + item["TenSach"].get<std::string>() + "</h5><div class='d-flex justify-content-center small text-warning mb-2'><div class='bi-star-fill'></div><div class='bi-star-fill'></div><div class='bi-star-fill'></div><div class='bi-star-fill'></div><div class='bi-star-fill'></div></div>Số lượng sách: " + std::to_string(item["SoLuong"].get<int>()) + "</div></div><div class='card-footer p-4 pt-0 border-top-0 bg-transparent'><div class='text-center'><a class='btn btn-outline-dark mt-auto' href='./"+ cccd + "/" + std::to_string(item["IDsach"].get<int>()) + "'>Mượn Sách Này</a></div></div></div></div>";
            }
            crow::mustache::context ctx({ {"ten", ten}, {"ThuVienLoad", html} });
            return page.render(ctx);
        }
        else {
            auto page = crow::mustache::load("error.html");
            return page.render();
        }
            });
    CROW_ROUTE(app, "/quanly/<string>/<string>/themsach/<string>/<int>")([&](const crow::request& req, std::string idquanly, std::string maquanly, std::string tensach, int soluong) {
        bool authenticated = false;
        std::string quanly_path = "quanly.json";
        std::string sach_path = "sach.json";

        json quanly_data, sach_data;
        std::ifstream quanly_file(quanly_path);
        std::ifstream sach_file(sach_path);
        quanly_file >> quanly_data;
        sach_file >> sach_data;
        for (auto& quanly : quanly_data) {
            if (quanly["IDQuanLy"] == idquanly && quanly["MaQuanLy"] == maquanly) {
                authenticated = true;
                break;
            }
        }

        if (!authenticated) {
            return crow::response(401, "Unauthorized");
        }

        json new_sach;
        new_sach["IDsach"] = sach_data.size() + 1;
        new_sach["TenSach"] = url_decode(tensach);
        new_sach["SoLuong"] = soluong;
        sach_data.push_back(new_sach);

        std::ofstream sach_file_out(sach_path);
        sach_file_out << sach_data.dump(4);
        sach_file_out.close();

        return crow::response(200, "OK");
            });
    CROW_ROUTE(app, "/quanly/<string>/<string>/themsv/<string>/<string>/<string>")([&](const crow::request& req, std::string idquanly, std::string maquanly, std::string soCCCC, std::string maSV, std::string TenSV) {

        bool authenticated = false;
        std::string quanly_path = "quanly.json";
        std::string sv_path = "sinhvien.json";

        json quanly_data, sach_data;
        std::ifstream quanly_file(quanly_path);
        std::ifstream sach_file(sv_path);
        quanly_file >> quanly_data;
        sach_file >> sach_data;
        for (auto& quanly : quanly_data) {
            if (quanly["IDQuanLy"] == idquanly && quanly["MaQuanLy"] == maquanly) {
                authenticated = true;
                break;
            }
        }

        if (!authenticated) {
            return crow::response(401, "Unauthorized");
        }

        json new_sv;
        new_sv["CCCD"] = url_decode(soCCCC);
        new_sv["MaSV"] = url_decode(maSV);
        new_sv["Ten"] = url_decode(TenSV);
        sach_data.push_back(new_sv);

        std::ofstream sach_file_out(sv_path);
        sach_file_out << sach_data.dump(4);
        sach_file_out.close();

        return crow::response(200, "OK");
        });
    CROW_ROUTE(app, "/thuvien/<string>/<string>/<int>")
        ([](const crow::request& req, std::string MaSV, std::string CCCD, int IDSach) {

        std::ifstream sach_file("sach.json");
        json sach_data;
        sach_file >> sach_data;

        std::string TenSach = "";
        for (auto& sach : sach_data.items()) {
            if (sach.value()["IDsach"] == IDSach) {
                TenSach = sach.value()["TenSach"];
                break;
            }
        }

        if (TenSach.empty()) {

            return crow::response(400, "Khong tim thay thong tin sach");
        }
        else {

            std::ifstream sinhvien_file("sinhvien.json");
            json sinhvien_data;
            sinhvien_file >> sinhvien_data;


            std::string TenSV = "";
            for (auto& sv : sinhvien_data.items()) {
                if (sv.value()["MaSV"] == MaSV && sv.value()["CCCD"] == CCCD) {
                    TenSV = sv.value()["Ten"];
                    break;
                }
            }

            if (TenSV.empty()) {

                return crow::response(400, "Khong tim thay thong tin sinh vien");
            }
            else {
                for (auto& sach : sach_data.items()) {
                    if (sach.value()["IDsach"] == IDSach) {
                        int so_luong = sach.value()["SoLuong"];
                        if (so_luong > 0) {
                            sach.value()["SoLuong"] = so_luong - 1;

                            json sachmuon_data;
                            sachmuon_data["MaSVMuon"] = MaSV;
                            sachmuon_data["TenSVMuon"] = TenSV;
                            sachmuon_data["IDvaSach"] = "ID: " + std::to_string(IDSach) + " - " + TenSach;

                            std::ifstream sachmuon_file("sachmuon.json");
                            json sachmuon_list;
                            sachmuon_file >> sachmuon_list;

                            sachmuon_list.push_back(sachmuon_data);

                            std::ofstream output_file("sachmuon.json");
                            output_file << sachmuon_list.dump(4);
                            std::ofstream sach_output_file("sach.json");
                            sach_output_file << sach_data.dump(4);

                            crow::response res;
                            res.set_header("Content-Type", "text/html; charset=UTF-8");
                            res.body = "<title>Thư Viện HUBT - Mượn Sách</title> <script>alert('Đã mượn sách thành công! Bạn vui lòng đến thư viện và lấy sách đã mượn!');const url=window.location.href,paths=url.split('/').slice(3,6),newUrl=window.location.protocol+'//'+window.location.host+'/'+paths.join('/');window.location.href=newUrl</script>";
                            return res;
                        }
                    }
                    }
            }
        }
            });

        
    app.port(80).multithreaded().run();
}