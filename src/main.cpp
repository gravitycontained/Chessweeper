//qpl.hpp is available at my repository: https://github.com/DanielRabl/QPL
#include <qpl/qpl.hpp>

namespace info {
	constexpr qpl::f32 square_width = 60;
	constexpr qpl::f32 square_decrease = 4;
	constexpr qsf::rgb revealed_color = qsf::rgb(192, 192, 192);
	constexpr qsf::rgb hidden_color = qsf::rgb(220, 220, 220);
	constexpr qsf::rgb hover_color = qsf::rgb(255, 255, 255);
	constexpr std::array<qpl::vector2i, 4> neighbour_checks = { qpl::vector2i(0, -1),qpl::vector2i(-1, 0), qpl::vector2i(1, 0), qpl::vector2i(0, 1) };
	constexpr auto piece_texture_width = 200u;
	qpl::vector2u field_dim = { 24, 14 };
	constexpr qpl::f32 texture_scale = 65;

}

enum class piece_type : qpl::u16 {
	empty = 0,
	white_king,
	white_queen,
	white_bishop,
	white_horse,
	white_rook,
	white_pawn,
	black_king,
	black_queen,
	black_bishop,
	black_horse,
	black_rook,
	black_pawn,
};

sf::IntRect piece_to_rect(piece_type piece) {
	auto index = qpl::u32_cast(piece) - 1u;
	sf::IntRect rect;
	rect.top = (index / 6) * info::piece_texture_width;
	rect.left = (index % 6) * info::piece_texture_width;
	rect.width = info::piece_texture_width;
	rect.height = info::piece_texture_width;
	return rect;
}
struct pieces {

	sf::Sprite white_king;
	sf::Sprite white_queen;
	sf::Sprite white_bishop;
	sf::Sprite white_horse;
	sf::Sprite white_rook;
	sf::Sprite white_pawn;
	sf::Sprite black_king;
	sf::Sprite black_queen;
	sf::Sprite black_bishop;
	sf::Sprite black_horse;
	sf::Sprite black_rook;
	sf::Sprite black_pawn;

	void set_scale() {
		auto factor = info::texture_scale / info::piece_texture_width;
		this->white_king.setScale(factor, factor);
		this->white_queen.setScale(factor, factor);
		this->white_bishop.setScale(factor, factor);
		this->white_horse.setScale(factor, factor);
		this->white_rook.setScale(factor, factor);
		this->white_pawn.setScale(factor, factor);
		this->black_king.setScale(factor, factor);
		this->black_queen.setScale(factor, factor);
		this->black_bishop.setScale(factor, factor);
		this->black_horse.setScale(factor, factor);
		this->black_rook.setScale(factor, factor);
		this->black_pawn.setScale(factor, factor);
	}
	void create(const sf::Texture& texture) {
		this->white_king.setTexture(texture);
		this->white_queen.setTexture(texture);
		this->white_bishop.setTexture(texture);
		this->white_horse.setTexture(texture);
		this->white_rook.setTexture(texture);
		this->white_pawn.setTexture(texture);
		this->black_king.setTexture(texture);
		this->black_queen.setTexture(texture);
		this->black_bishop.setTexture(texture);
		this->black_horse.setTexture(texture);
		this->black_rook.setTexture(texture);
		this->black_pawn.setTexture(texture);

		this->white_king.setTextureRect(piece_to_rect(piece_type::white_king));
		this->white_queen.setTextureRect(piece_to_rect(piece_type::white_queen));
		this->white_bishop.setTextureRect(piece_to_rect(piece_type::white_bishop));
		this->white_horse.setTextureRect(piece_to_rect(piece_type::white_horse));
		this->white_rook.setTextureRect(piece_to_rect(piece_type::white_rook));
		this->white_pawn.setTextureRect(piece_to_rect(piece_type::white_pawn));
		this->black_king.setTextureRect(piece_to_rect(piece_type::black_king));
		this->black_queen.setTextureRect(piece_to_rect(piece_type::black_queen));
		this->black_bishop.setTextureRect(piece_to_rect(piece_type::black_bishop));
		this->black_horse.setTextureRect(piece_to_rect(piece_type::black_horse));
		this->black_rook.setTextureRect(piece_to_rect(piece_type::black_rook));
		this->black_pawn.setTextureRect(piece_to_rect(piece_type::black_pawn));

		this->set_scale();
	}
};

struct square {
	bool has_mine = false;
	bool is_revealed = false;
	bool is_hovering = false;
	bool checked = false;
	bool has_flag = false;
	qpl::u32 flag_sprite_index = 0u;
	qpl::u32 mine_neighbours = 0u;
	qpl::animation fade_animation;
	square() {
		this->fade_animation.set_duration(0.2);
	}
};

struct field {
	std::vector<square> squares;
	qpl::vector2f position = { info::square_width, info::square_width };
	qpl::vector2u field_dim;
	pieces pieces;
	sf::Texture flag_texture;
	qpl::size mine_spawn_count = 0;
	bool mines_generated = false;

	void create(qpl::vector2u dim) {
		this->field_dim = dim;
		this->squares.resize(dim.x * dim.y);
		this->rects.resize(dim.x * dim.y);
		this->pawn_sprites.resize(dim.x * dim.y);
		this->make_graphic();
	}

	void make_graphic() {
		for (qpl::u32 i = 0u; i < this->squares.size(); ++i) {
			const auto& square = this->squares[i];
			if (square.is_revealed) {
				this->rects[i].set_color(info::revealed_color);
			}
			else {
				this->rects[i].set_color(info::hidden_color);
			}
			auto y = i / this->field_dim.x;
			auto x = i % this->field_dim.x;
			auto pos = qpl::vec(x + 0.5, y + 0.5) * info::square_width + this->position;
			this->rects[i].set_dimension(qpl::vec(info::square_width, info::square_width) - info::square_decrease);
			this->rects[i].set_center(pos);
			this->pawn_sprites[i] = this->pieces.white_pawn;
			this->pawn_sprites[i].set_center(this->rects[i].get_center());
			//this->pawn_sprites[i].set_color(qsf::rgb::white.with_alpha(100));
		}
	}

	void flag_square(qpl::u32 i) {
		auto& square = this->squares[i];
		auto& rect = this->rects[i];
		if (!square.has_flag) {
			square.has_flag = true;
			square.flag_sprite_index = this->flag_sprites.size();
			this->flag_sprites.push_back({});
			auto& sprite = this->flag_sprites.back();
			sprite.set_texture(this->flag_texture);
			sprite.set_scale(info::texture_scale / 550);
			sprite.set_center(rect.get_center());
			this->pawn_sprites[i].set_color(qsf::rgb::transparent);
		}
		else {
			square.has_flag = false;
			this->flag_sprites[square.flag_sprite_index].set_color(qsf::rgb::transparent);
			this->pawn_sprites[i].set_color(qsf::rgb::white);
		}
	}
	void reveal_square(qpl::u32 i, bool recursive = true) {
		auto& square = this->squares[i];
		auto& rect = this->rects[i];
		square.is_revealed = true;
		square.checked = true;

		qpl::i32 x = i % this->field_dim.x;
		qpl::i32 y = i / this->field_dim.x;


		if (recursive && !square.mine_neighbours) {
			for (qpl::i32 iy = -1; iy <= 1; ++iy) {
				for (qpl::i32 ix = -1; ix <= 1; ++ix) {
					auto dx = x + ix;
					auto dy = y + iy;

					bool is_neighbour_check = false;
					for (auto& i : info::neighbour_checks) {
						if (i == qpl::vector2u(ix, iy)) {
							is_neighbour_check = true;
						}
					}
					if (dx >= 0 && dx < this->field_dim.x && dy >= 0 && dy < this->field_dim.y) {
						auto index = dx + this->field_dim.x * dy;

						if (is_neighbour_check) {
							if (!this->squares[index].checked && !this->squares[index].has_mine && !this->squares[index].has_flag) {
								reveal_square(index);
							}
						}
						else {
							if (!this->squares[index].checked && this->squares[index].mine_neighbours) {
								reveal_square(index);
							}
						}
					}
				}
			}
		}

		this->pawn_sprites[i].set_color(qsf::rgb::transparent);
		if (square.has_mine) {
			rect.set_color(qsf::rgb::red);
		}
		else {
			rect.set_color(info::revealed_color);
		}
		if (square.has_flag) {
			square.has_flag = false;
			this->flag_sprites[square.flag_sprite_index].set_color(qsf::rgb::transparent);
		}

		if (square.mine_neighbours) {
			this->texts.push_back({});
			auto& text = this->texts.back();
			text.set_font("sweeper");
			text.set_character_size(38);
			text.set_string(qpl::to_string(square.mine_neighbours));
			text.set_center(rect.get_center());

			switch (square.mine_neighbours) {
			case 1u: text.set_color(qsf::rgb(2, 20, 253)); break;
			case 2u: text.set_color(qsf::rgb(1, 126, 20)); break;
			case 3u: text.set_color(qsf::rgb(254, 0, 0)); break;
			case 4u: text.set_color(qsf::rgb(1, 1, 128)); break;
			case 5u: text.set_color(qsf::rgb(126, 3, 3)); break;
			case 6u: text.set_color(qsf::rgb(0, 128, 128)); break;
			case 7u: text.set_color(qsf::rgb(0, 0, 0)); break;
			case 8u: text.set_color(qsf::rgb(128, 128, 128)); break;
			}
		}
		if (square.has_mine) {
			this->black_queen_sprites.push_back({});
			auto& sprite = this->black_queen_sprites.back();
			sprite = this->pieces.black_queen;
			sprite.set_center(rect.get_center());
		}

	}
	void reveal_all() {
		for (qpl::u32 i = 0u; i < this->squares.size(); ++i) {
			this->reveal_square(i, false);
		}
	}
	void update(const qsf::event_info& info) {
		for (qpl::u32 i = 0u; i < this->squares.size(); ++i) {
			auto& square = this->squares[i];
			auto& rect = this->rects[i];
			if (!square.is_revealed) {
				auto hitbox = rect.get_hitbox();
				auto hovering = hitbox.increased(info::square_decrease / 2).contains(info.mouse_position());
				if (hovering && info.left_mouse_clicked()) {
					if (!this->mines_generated) {
						this->add_mines(i);
					}
					for (auto& i : this->squares) {
						i.checked = false;
					}
					this->reveal_square(i);
					if (square.has_mine) {
						this->reveal_all();
					}
					continue;
				}
				if (hovering && info.right_mouse_clicked()) {
					this->flag_square(i);
					if (!square.has_flag) {
						square.fade_animation.go_forwards();
						square.is_hovering = true;
					}
				}
				if (square.has_flag) {
					rect.set_color(info::revealed_color);
				}
				else {
					if (hovering) {
						if (!square.is_hovering) {
							square.fade_animation.go_forwards();
						}
						square.is_hovering = true;
					}
					else if (square.is_hovering) {
						square.is_hovering = false;
						square.fade_animation.go_backwards();
					}

					square.fade_animation.update();
					if (square.fade_animation.is_running()) {
						auto progress = square.fade_animation.get_curve_progress(2.0);
						rect.set_color(info::hidden_color.interpolated(info::hover_color, progress));
					}
				}
			}
		}
	}
	void add_mines(qpl::u32 index) {
		this->mines_generated = true;
		auto count = this->mine_spawn_count;

		{
			qpl::i32 x = index % this->field_dim.x;
			qpl::i32 y = index / this->field_dim.x;
			while (count) {
				auto random = qpl::random(this->squares.size() - 1);

				qpl::i32 rx = random % this->field_dim.x;
				qpl::i32 ry = random / this->field_dim.x;
				if (qpl::abs(x - rx) <= 1 && qpl::abs(y - ry) <= 1) {
					continue;
				}

				if (!this->squares[random].has_mine) {
					this->squares[random].has_mine = true;
					--count;
				}
			}
		}

		for (qpl::u32 i = 0u; i < this->squares.size(); ++i) {
			qpl::i32 x = i % this->field_dim.x;
			qpl::i32 y = i / this->field_dim.x;

			if (!this->squares[i].has_mine) {
				qpl::u32 count = 0u;
				for (qpl::i32 iy = -1; iy <= 1; ++iy) {
					for (qpl::i32 ix = -1; ix <= 1; ++ix) {
						auto dx = x + ix;
						auto dy = y + iy;
						if (dx >= 0 && dx < this->field_dim.x && dy >= 0 && dy < this->field_dim.y) {
							auto index = dx + this->field_dim.x * dy;

							if (this->squares[index].has_mine) {
								++count;
							}
						}
					}
				}
				this->squares[i].mine_neighbours = count;
			}
		}
	}
	qsf::rectangles rects;
	std::vector<qsf::text> texts;
	std::vector<qsf::sprite> black_queen_sprites;
	std::vector<qsf::sprite> pawn_sprites;
	std::vector<qsf::sprite> flag_sprites;

	

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->rects);
		draw.draw(this->texts);
		draw.draw(this->black_queen_sprites);
		draw.draw(this->pawn_sprites);
		draw.draw(this->flag_sprites);
	}
};

struct main_state : qsf::base_state {
	void init() override {
		this->get_texture("pieces").generateMipmap();
		this->get_texture("pieces").setSmooth(true);

		qsf::load_texture(this->field.flag_texture, "resources/flag.png");
		this->field.pieces.create(this->get_texture("pieces"));
		this->field.create(info::field_dim);
		this->field.mine_spawn_count = 80;
		this->clear_color = qsf::rgb(128, 128, 128);

	}
	void updating() override {
		this->update(this->field);
	}
	void drawing() override {
		this->draw(this->field);
	}
	field field;
};

int main() {
	auto dim = (info::field_dim + 2) * info::square_width;
	qsf::framework framework;
	framework.set_title("QPL");
	framework.set_dimension(dim);
	framework.add_font("sweeper", "resources/mine-sweeper.ttf");
	framework.add_texture("pieces", "resources/pieces.png");

	framework.add_state<main_state>();
	framework.game_loop();
}